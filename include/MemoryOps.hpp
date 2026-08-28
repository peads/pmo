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
#include <iostream>
#include "types/PointerUnion.hpp"
#include "types/PseudoContainer.hpp"
// #if defined(_WIN32) || defined(_WIN64)
// #include <windows/MemoryOps.hpp>
// #endif

namespace PMO
{
    struct Pattern
    {
        PointerUnion pattern;
        PointerUnion mask;
        PointerUnion code;
        const size_t patternLen;
        const size_t maskLen;
        const size_t codeLen;
        template <typename T, size_t N, typename U, size_t M, typename V, size_t P>
        explicit Pattern(T (&pattern)[N], U (&mask)[M], V (&code)[P])
            : pattern(pattern), mask(mask), code(code),
                patternLen(N-1), maskLen(M), codeLen(P-1)
        {}
    };

    /**
    * @brief          Scan through memory represented by mInfo for all occurrences of given
    *                   pattern with its associated mask.
    * @param ptr      Base address of module.
    * @param size     Size of Image.
    * @param pattern  Contiguous memory space containing bytes of the search key
    *                   (e.g. an array of (u)int8s, an escape sequence like "\xDE\xAD\xBE\xEF", or
    *                   pointer to a (u)int *ptr->0xEFBEADDE [N.B. endianness, and pattern length
    *                   restriction to size of integral type]).
    * @param mask     null-terminated, wildcard-compatible sequence of characters (i.e. a cstring)
    *                   that can contain either an 'x' (requirement indicator), or
    *                   '?' (wildcard indicator) per byte or per nibble (i.e. for each hexit).
    *                   For example, given pattern "\xDE\xAD\xBE\xEF", a mask like "xx?x" could be
    *                   used for per byte resolution, or a mask like "xxxx??xx" for per octit
    *                   resolution; they are effectively equivalent.
    * @param patternLength Length of the pattern because it is not expected to be null-terminated,
    *                       and may be different from the length of the null-terminated mask
    *                       allowing for automatic resolution selection.
    * @param out        Reference to a pseudo-conformant STL Container type (e.g. std::vector)
    */
    template <PseudoContainer T>
    static void findPatterns(
        const void *ptr,
        const size_t size,
        const char *pattern,        // needn't be null-terminated (in fact, shouldn't be)
        const char *mask,           // must be null-terminated
        const size_t patternLength,
        T &out
    )
    {
        //        const uint64_t hitMask = (1ULL << (patternLength << 3)) - 1;
        const size_t maskLength = strlen(mask);
        if (patternLength > maskLength)
            return;
        const bool isFine = maskLength > patternLength;
        const uint8_t incre = 1 + isFine;
        const uint8_t byteMask = isFine ? 0xF : 0xFF;

        PointerUnion base = {ptr};
        PointerUnion stack = {mask};
        PointerUnion patternU = {pattern};

        for (const size_t endOffset = size + base.address;
             base.address < endOffset; ++base.cptr)
        {
            bool isHit = true;
            uint64_t hits = 0;
            size_t j = 0;
            for (; *stack.cptr != '\0'; ++j)
            {
                for (uint8_t k = 0; k < incre; ++k)
                {
                    const uint8_t shift = k << 2;
                    const uint8_t a = *(stack.cptr + k);
                    const uint8_t b = byteMask & *patternU.cptr >> shift;
                    const uint8_t c = byteMask & *(base.cptr + j) >> shift;
                    hits += isHit &= a == '?' || b == c;
                }
                if (!isHit)
                    break;
                patternU.cptr++;
                stack.cptr += incre;
            }
            if (isHit)
            {
                out.push_back(base.address);
                base.cptr += j;
            }
            stack.ptr = mask;
            patternU.ptr = pattern;
        }
    }
}
#endif //MEMORYOPS_HPP
