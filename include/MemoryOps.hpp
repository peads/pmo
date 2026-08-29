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
#include <bmi2.hpp>

#include "types/Pattern.hpp"
#include "parse/ParseJmp.hpp"

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
    inline uintptr_t findNamedFunction(uintptr_t &addr, T out)
    {
        const uintptr_t result = startParseJmp(addr);
        addr += 7 + result;
        *out = *reinterpret_cast<T>(addr);
        return result;
    }

    /**
    * Same as above, but doesn't clobber first operand.
    */
    inline uintptr_t findNamedFunction(uintptr_t addr, uintptr_t *out)
    {
        const uintptr_t result = startParseJmp(addr);
        if (result)
        {
            addr += 7 + result;
        }
        *out = addr;
        return result;
    }

    template <PseudoContainer T>
    inline bool findPatterns(const uintptr_t addr, const size_t len, const Pattern &pattern, T &out)
    {
        bool result = false;

        const auto &pMask = *pattern.searchMask;
        auto &bMask = *pattern.bitMask;
        const auto pSize = pMask.size();

        auto *ptr = reinterpret_cast<uint8_t*>(addr);
        for (; ptr && reinterpret_cast<uintptr_t>(ptr) < len + addr - 8; ptr += 8)
        {
            uint64_t notHit = 0;
            const auto baseAddr = reinterpret_cast<uint64_t*>(ptr);
            for (size_t i = 0; i < pSize; ++i)
            {
                if (!*baseAddr)
                {
                    notHit = 1;
                    break;
                }
                const auto val = *(baseAddr + i) & pMask[i];
                const auto msk = *(reinterpret_cast<uint64_t*>(bMask.data()) + i);
                const auto pat = *(reinterpret_cast<uint64_t*>(pattern.pattern.address) + i) & pMask[i];
                notHit |= pat ^ (pdep(pext(val, msk), msk) | val);
            }
            if (!notHit)
            {
                result = true;
                out.push_back(reinterpret_cast<uintptr_t>(baseAddr));
            }
        }
        return result;
    }
}
#endif //MEMORYOPS_HPP
