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

#include "types/pattern/Pattern.hpp"
#include "parse/ParseJmp.hpp"

#if defined(__clang__)
    #define PRAGMA_IVDEP _Pragma("clang loop vectorize(enable) interleave(enable)")
    #define PRAGMA_UNROLL(n) _Pragma("clang loop unroll_count(n)")
#elif defined(__GNUC__) || defined(__GNUG__)
    #define PRAGMA_IVDEP _Pragma("GCC ivdep")
    #define PRAGMA_UNROLL(n) _Pragma("GCC unroll n")
#elif defined(_MSC_VER) // MSVC
    #define PRAGMA_IVDEP _Pragma("loop(ivdep)")
    #define PRAGMA_UNROLL(n) // MSVC doesn't have a direct loop unroll factor pragma
#else
    #define PRAGMA_IVDEP
    #define PRAGMA_UNROLL(n)
#endif

namespace PMO
{
#if defined(__aarch64__) || defined(_M_ARM64)
    inline uint32_t parseLdrImm(const uintptr_t addr) noexcept
    {
        uint32_t* ptr = (uint32_t*)addr;
        if (((*ptr >> 27) & 7) == 7) // is ldr unsigned scaled imm?
        {
            return ((*ptr >> 10) & 0x3FF) << ((*ptr >> 30) & 3);
        }
        return 0; // it wasn't
    }

    //30 04 00 90 10 A6 43 F9 00 02 1F D6
    inline int32_t parseAdrpImm(uintptr_t addr) noexcept
    {
        uint32_t* ptr = (uint32_t*)addr;
        if ((*ptr >> 31) & 1) // ADRP?
        {
            const uint32_t immhi = (*ptr & 0x1FFF'FFFF) >> 5;
            const uint32_t immlo = (*ptr & 0x6000'0000) >> 29;
            return ((int32_t)(((immhi << 2) | immlo) << 11) >> 11) << 12;
        }
        return 0; // it wasn't
    }

    inline uintptr_t findNamedFunction(uintptr_t addr, uintptr_t* out) noexcept
    {
        if (!addr) return 0;
        uint32_t ins = *(uint32_t*)addr;
        uint32_t foo = ins >> 26;
        foo &= 0b01'1111;
        if (foo != 0b101)
            return 0;
        int32_t imm = (int32_t)(ins << 6) >> 6;
        uintptr_t result = 0;
        if (imm)
        {
            addr += (int64_t)imm << 2;
            int32_t adrpImm = parseAdrpImm(addr);
            uint32_t ldrImm = parseLdrImm(addr + 4);
            addr &= ~0xFFF; // align to nearest 4k page boundary; side-effect: yeet 12 lowest bits
            addr += (int64_t)adrpImm;
            addr += (uint64_t)ldrImm;
            result += (int64_t)adrpImm;
            result += (uint64_t)ldrImm;
        }
        *out = addr;
        return result;
    }

    template <typename T, typename =
              std::enable_if_t<std::is_pointer_v<T> // is ptr to *non-member* fn ptr
                  && std::is_function_v<std::remove_pointer_t<std::remove_pointer_t<T>>>>>
        inline uintptr_t findNamedFunction(uintptr_t addr, T out) noexcept
    {
        uintptr_t addr1;
        const uintptr_t result = findNamedFunction(addr, &addr1);
        if (result)
        {
            *out = *reinterpret_cast<T>(addr1);
            addr = addr1;
        }
        return result;
    }
#else
    /**
    * @brief    Parses thunk at given address for the address values and function pointer to return.
    * @details  Takes reference to the address of a known thunk function and returns RVA, VA and
    *             function pointer of inferred-type to the address to which the jmp redirects The
    *             value of the given address reference is mutated to the RVA, the second operand
    *             stores the function pointer, and the VA is returned. The address result is parsed
    *             from bytes of the jmp stored at the given address [N.B. Only jmp \em far,
    *             \em absolute \em indirect (i.e. FF /5, and REX.W FF /5) is supported].
    * @param[in]    addr   Given address of thunk function; reused to store VA.
    * @param[out]   out    Pointer to storage for resultant function pointer, of inferred type,
    *                         cast from VA.
    * @return              RVA
    */
    template <typename T, typename =
              std::enable_if_t<std::is_pointer_v<T> // is ptr to *non-member* fn ptr
                  && std::is_function_v<std::remove_pointer_t<std::remove_pointer_t<T>>>>>
    inline uintptr_t findNamedFunction(uintptr_t addr, T out) noexcept
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
    * Same as above, but out arg is uintptr_t.
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
#endif

    struct SearchContext
    {
        const PointerUnion theEnd;
        const size_t offset;
        PointerUnion &pointer;
        size_t &len;
        Pattern &searchStruct;
        bool &result;
    };

    typedef bool (*searchFunc)(SearchContext &ctx);

    inline bool searchChunked(SearchContext &ctx) noexcept
    {
        uint64_t notHit = -1;
        auto &[theEnd, offset, pointer, len, searchStruct, result] = ctx;
        auto baseAddr = pointer.u64ptr;
        auto pat = searchStruct.pattern.u64ptr;

        for (auto pmsk = (uint64_t*)searchStruct.pmsk().data(),
            bmsk = (uint64_t*)searchStruct.bmsk().data(); len - 7 > 0 &&
            pat < theEnd.u64ptr; ++pmsk, ++bmsk, ++pat)
        {
            const auto valMasked = *baseAddr & *pmsk | *bmsk;
            const auto patMasked = *pat & *pmsk | *bmsk;
            notHit = valMasked ^ patMasked;
            if (notHit)
            // if ((notHit = (*baseAddr ^ *pat) & (~bmsk->i & pmsk->i)))
            {
                --len;
                break;
            }
            ++baseAddr;
        }

        if (!notHit)
        {
            result = true;
            searchStruct.push_back(pointer.address + offset);
            // pointer.u64ptr = baseAddr;
            pointer.u8ptr += searchStruct.patternLen;
            len -= searchStruct.patternLen;
        }
        ++pointer.u8ptr;
        return result;
    }

    inline bool searchBytewise(SearchContext &ctx) noexcept
    {
        bool notHit = true;
        auto &[theEnd, offset, pointer, len, searchStruct, result] = ctx;
        for (auto pat = searchStruct.pattern.cptr,
                  msk = searchStruct.mask.cptr,
                  val = pointer.cptr; pat < theEnd.cptr; ++pat, ++msk, ++val)
            if ('?' != *msk && ((notHit = *pat ^ *val)))
                break;

        if (notHit)
            ++pointer.cptr;
        else
        {
            searchStruct.push_back(offset + pointer.address);
            result = true;
            pointer.cptr += searchStruct.patternLen;
        }
        return result;
    }

    // no it doesn't. ReSharper can't even reference types in structs
    // ReSharper disable once CppDFAConstantFunctionResult
    inline bool findPatterns(const uintptr_t addr, size_t len,
                             Pattern &searchStruct,
                             const bool stopOne = false,
                             const searchFunc search = searchChunked) noexcept
    {
        if (!(addr && len))
            return false;

        bool result = false;
        const PointerUnion theEnd = {.address = searchStruct.pattern.address + searchStruct.patternLen};
        PointerUnion pointer = {.address = addr};

        SearchContext ctx{
            .theEnd = theEnd,
            .offset = 0,
            .pointer = pointer,
            .len = len,
            .searchStruct = searchStruct,
            .result = result,
        };

        while (pointer.address < len + addr)
        {
            if (search(ctx) && stopOne)
                break;
        }
        return result;
    }
}
#endif //MEMORYOPS_HPP
