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
#ifndef PMO_BITMANIP_H
#define PMO_BITMANIP_H

#include <cstdint>
//#include <avx512.h>

inline uint64_t software_pdep(const uint64_t a, uint64_t b)
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

inline uint64_t software_pext(const uint64_t a, uint64_t b)
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
#ifdef pdep
#undef pdep
#endif
#ifdef pext
#undef pext
#endif
#if (defined(__BMI2__) || defined(_MSC_VER) && defined(__AVX2__))
#include <immintrin.h>
#define pdep(a,b) _pdep_u64(a,b)
#define pext(a,b) _pext_u64(a,b)
#elif  defined(__ARM_FEATURE_SVE2) //TODO this may not be the correct usage
#include <arm_sve.h>
#define pdep(a,b) svbdep_n_u64(a, b)
#define pext(a,b) svbext_n_u64(a, b)
#elif defined(_ARCH_PWR10)
#define pdep(a,b) __builtin_pdepd(a, b)
#define pext(a,b) __builtin_pextd(a, b)
#else
#define pdep(a,b) software_pdep(a,b)
#define pext(a,b) software_pext(a,b)
#endif
#endif //PMO_BITMANIP_H
