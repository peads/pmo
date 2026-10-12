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
#ifndef PMO_DEFINES_HPP
#define PMO_DEFINES_HPP
#if defined(__clang__)
    #define PRAGMA_IVDEP _Pragma("clang loop vectorize(enable) interleave(enable)")
    #define PRAGMA_UNROLL _Pragma("clang loop unroll_count(8)")
#elif defined(__GNUC__) || defined(__GNUG__)
    #define PRAGMA_IVDEP _Pragma("GCC ivdep")
    #define PRAGMA_UNROLL _Pragma("GCC unroll 8")
#elif defined(_MSC_VER) // MSVC
    #define PRAGMA_IVDEP _Pragma("loop(ivdep)")
    #define PRAGMA_UNROLL // MSVC doesn't have a direct loop unroll factor pragma
#else
    #define PRAGMA_IVDEP
    #define PRAGMA_UNROLL
#endif

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
    #define FORCE_INLINE_LAMBDA __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define FORCE_INLINE_LAMBDA [[msvc::forceinline]]
#else
    #define FORCE_INLINE_LAMBDA
#endif
#endif //PMO_DEFINES_HPP
