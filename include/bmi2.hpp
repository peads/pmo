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
#ifndef BMI2_HPP
#define BMI2_HPP
#include <cstdint>

#if (defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)) && defined(__BMI2__) && __has_include(<immintrin.h>)
#include <immintrin.h>
    inline uint64_t (*pdep)(uint64_t, uint64_t) = _pdep_u64;
    inline uint64_t (*pext)(uint64_t, uint64_t) = _pext_u64;
#elif defined(_MSC_VER) || !(defined(__x86_64__) || defined(_M_X64)) && !defined(__BMI2__)
    inline uint64_t pdep(uint64_t a, uint64_t b)
    {
        uint64_t r = 0, m = 1;
        while (b)
        {
            const uint64_t l = b & (0 - b);
            b = b ^ l;
            const uint64_t s = 0 - (a & m);
            r = r | (l & s);
            m = m + m;
        }
        return r;
    }

    inline uint64_t pext(uint64_t a, uint64_t b)
    {
        uint64_t r = 0;
        for (uint64_t m = 1; b; b &= b - 1, m <<= 1)
        {
            if (a & b & (0 - b))
            {
                r |= m;
            }
        }
        return r;
    }
#else
extern "C" {
    uint64_t pDep(uint64_t a, uint64_t b);
    uint64_t pExt(uint64_t a, uint64_t b);
}
    inline uint64_t (*pdep)(uint64_t, uint64_t) = pDep;
    inline uint64_t (*pext)(uint64_t, uint64_t) = pExt;
#endif
#endif // BMI2_HPP
