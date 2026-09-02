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

// aside from the obvious exclusions you do not want pext, or pdep running on amd zen1s or zen2s
#if !(defined(__x86_64__) || defined(_M_X64)) || defined(__znver2__) || defined(__znver1__)
    #include <cstdint>
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
#elif (defined(__BMI2__) || defined(_MSC_VER) && defined(__AVX2__)) \
    && __has_include(<immintrin.h>)                       // ah yes, definitely AVX2 == BMI2.
                                                          // surely, there's no need to differentiate
                                                          // them; they're exactly the same
                                                          // instruction set extension. thank you, msvc.
                                                          // at least, this will come in handy later.
#include <immintrin.h>
#define pdep(a,b) _pdep_u64(a,b)
#define pext(a,b) _pext_u64(a,b)
#elif defined(__BMI2__) &&  !__has_include(<immintrin.h>) // __BMI2__ implicitly excludes msvc
#include <cstdint>
inline uint64_t pdep(uint64_t a, uint64_t b)
{
    __asm__(
            ".intel_syntax noprefix\n\t"  // Fuck ATT syntax
            "pdep   %0, %0, %1\n\t"
            ".att_syntax"                 // Fine. You can have it back ig
            : "+r"(a)                     // Output/Bidirectional operand(s)
            : "r"(b)                      // Input operand(s)
            : "cc"                        // Clobber(s)
    );
    return a;
}

inline uint64_t pext(uint64_t a, uint64_t b)
{
    __asm__(
            ".intel_syntax noprefix\n\t"  // Fuck ATT syntax
            "pext   %0, %0, %1\n\t"
            ".att_syntax\n\t"             // Fine. You can have it back ig
            : "+r"(a)                     // Output/Bidirectional operand(s)
            : "r"(b)                      // Input operand(s)
            : "cc"                        // Clobber(s)
    );
    return a;
}
#else // surely, this case cannot be possible,
      // but if it somehow is you'll have to assemble
      // and link src/bmi2.asm. God speed to you,
      // and using w/e compiler it is that you're using
#include <cstdint>
extern "C" {
    uint64_t pDep(uint64_t a, uint64_t b);
    uint64_t pExt(uint64_t a, uint64_t b);
}
#define pdep(a,b) pDep(a,b)
#define pext(a,b) pExt(a,b)
#endif

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
    #define FORCE_INLINE_LAMBDA __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define FORCE_INLINE_LAMBDA [[msvc::forceinline]]
#else
    #define FORCE_INLINE_LAMBDA
#endif
#endif // BMI2_HPP
