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

#if (defined(__x86_64__) || defined(_M_X64)) && (defined(__BMI2__) || (defined(_MSC_VER) && defined(__AVX2__)))
extern "C" {
    uint64_t pDep(uint64_t a, uint64_t b);

    uint64_t pExt(uint64_t a, uint64_t b);
}
#else
inline uint64_t pDep(const uint64_t a, uint64_t b)
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

inline uint64_t pExt(uint64_t a, uint64_t b)
{
    uint64_t r = 0;
    for (uint64_t m = 1; b; b &= b - 1, m <<= 1)
    {
        if (a & b & -b)
        {
            r |= m;
        }
    }
    return r;
}
#endif
typedef uint64_t (*bmi2Fn)(uint64_t, uint64_t);

inline bmi2Fn pdep = pDep;
inline bmi2Fn pext = pExt;
#endif // BMI2_HPP
