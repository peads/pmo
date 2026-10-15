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

template <typename T>
T software_pdep(const T a, T b) requires std::is_unsigned_v<T>
{
    T r = 0, m = 1;
    while (b)
    {
        const T l = b & (0 - b);
        b = b ^ l;
        const T s = 0 - (a & m);
        r = r | (l & s);
        m = m + m;
    }
    return r;
}

template <typename T>
T software_pext(const T a, T b) requires std::is_unsigned_v<T>
{
    T r = 0;
    for (T m = 1; b; b &= b - 1, m <<= 1)
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
// #define pext32(a,b) _pext_u32(a,b)
// #define pdep32(a,b) _pdep_u32(a,b)
#elif (defined(__ARM_FEATURE_SVE) && defined(__ARM_FEATURE_SVE2_BITPERM)) || \
    (defined(__ARM_FEATURE_SME) && defined(__ARM_FEATURE_SSVE_BITPERM))
#include <arm_sve.h>
#define pdep(a,b) svclastb_n_u64(svpfalse(), 0, svbdep_n_u64(svdup_n_u64(a), b))
#define pext(a,b) svclastb_n_u64(svpfalse(), 0, svbext_n_u64(svdup_n_u64(a), b))
#elif defined(_ARCH_PWR10)
#define pdep(a,b) __builtin_pdepd(a, b)
#define pext(a,b) __builtin_pextd(a, b)
#else
#define pdep(a,b) software_pdep(a,b)
#define pext(a,b) software_pext(a,b)
// #define pdep32(a,b) pdep(a,b)
// #define pext32(a,b) pext(a,b)
#define BMANIP_SW_ONLY 1
#endif
#endif //PMO_BITMANIP_H
