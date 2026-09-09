/*
 * This file is part of the pmo (peads Memory Operations) distribution
 * (https://github.com/peads/pmo).
 * Copyright (c) 2026 Patrick Eads.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MEMORYOPS_HPP
#define MEMORYOPS_HPP

#include "types/Pattern.hpp"
#include "parse/ParseJmp.hpp"
#include <limits>
#define STREAM_LEN (1 << 21)

namespace PMO
{
    /**
    * @brief    Parses thunk at given address for the address values and function pointer to return.
    * @details  Takes reference to the address of a known thunk function and returns RVA, VA and
    *             function pointer of inferred-type to the address to which the jmp redirects The
    *             value of the given address reference is mutated to the RVA, the second operand
    *             stores the function pointer, and the VA is returned. The address result is parsed
    *             from bytes of the jmp stored at the given address [N.B. Only jmp \em far,
    *             \em absolute \em indirect (i.e. FF /5, and REX.W FF /5) is supported].
    * @param[in,out] addr   Given address of thunk function; reused to store VA.
    * @param[out] out       Pointer to storage for resultant function pointer, of inferred type,
    *                         cast from VA.
    * @return               RVA
    */
    template <typename T, typename =
              std::enable_if_t<std::is_pointer_v<T> // is ptr to *non-member* fn ptr
                  && std::is_function_v<std::remove_pointer_t<std::remove_pointer_t<T>>>>>
    inline uintptr_t findNamedFunction(uintptr_t &addr, T out) noexcept
    {
        const uintptr_t result = startParseJmp(addr);
        if (result)
        {
            addr += 7 + result;
            *out = *reinterpret_cast<T>(addr);
        }
        return result;
    }

    /**
    * Same as above, but doesn't clobber first operand.
    */
    inline uintptr_t findNamedFunction(uintptr_t addr, uintptr_t *out) noexcept
    {
        const uintptr_t result = startParseJmp(addr);
        if (result)
        {
            addr += 7 + result;
        }
        *out = addr;
        return result;
    }

    inline bool findPatterns(const uintptr_t addr, size_t len, Pattern &searchStruct,
        const uint64_t offset = 0, const bool stopOne = false) noexcept
    {
        bool result = false;

        static uint32_t pCnt = -1;
        static std::string smallest;

        const auto pend = searchStruct.pattern.u64ptr + searchStruct.pSize;
        for (auto *ptr = reinterpret_cast<uint8_t*>(addr);
             ptr && reinterpret_cast<uintptr_t>(ptr) < len + addr;)
        {
            uint64_t notHit = -1;
            size_t shift = 1;

            auto baseAddr = reinterpret_cast<uint64_t*>(ptr);
            auto pat = searchStruct.pattern.u64ptr;
            for (auto pmsk = searchStruct.pmsk().data(),
                bmsk = searchStruct.bmsk().data();
                pat < pend; ++pmsk, ++bmsk, ++pat)
            {
                const auto val = *baseAddr & *pmsk;
                const auto valMasked = val | *bmsk;
                const auto patMasked = *pat & *pmsk | *bmsk;
                if ((notHit = valMasked ^ patMasked))
                {
                    shift = std::countr_zero(notHit) >> 3;
                    shift = shift < 1 ? 1 : shift;
                    break;
                }
                ++baseAddr;
            }

            if (notHit)
            {
                ptr += shift;   // xx[xxxxxxxx]x ... xx0
                len -= shift;   // xxx[xxxxxxxx] ... xx0
                                // xxx[xxxxxxxx]x ... x0
            }
            else
            {
                result = true;
                searchStruct.push_back(reinterpret_cast<uintptr_t>(ptr) + offset);
                if (stopOne)
                    break;
                ptr += 8;   // [yyyyyyyy]xxxxxxxx
                            // yyyyyyyy[xxxxxxxx]
            }
        }
        return result;
    }
}
#endif //MEMORYOPS_HPP
