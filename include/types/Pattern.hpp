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
#ifndef PATTERN_HPP
#define PATTERN_HPP

#include "PointerUnion.hpp"
#include "SetWrapper.hpp"
#include "PseudoVector.hpp"

namespace PMO
{
    struct Pattern
    {
        const size_t patternLen;
        const size_t maskLen;
        const size_t codeLen;
        const PointerUnion pattern;
        const PointerUnion mask;
        const PointerUnion code;

        private:
            size_t pSize;
            PseudoVector<uint64_t> bMask;
            PseudoVector<uint64_t> pMask;
            SetWrapper<uintptr_t> m_occurrences;

        public:
            PseudoVector<uint64_t> *const bitMask;
            PseudoVector<uint64_t> *const searchMask;
            SetWrapper<uintptr_t> *const occurrences;

            template <size_t N, size_t M, size_t P>
            explicit Pattern(const char (&pattern)[N], const char (&mask)[M], const char (&code)[P])
                : patternLen(N - 1),
                  maskLen(M),
                  codeLen(P - 1),
                  pattern{.str = pattern},
                  mask{.str = mask},
                  code{.str = code},
                  // pSize(!(N % 8) ? N >> 3 : 1 + (N >> 3)),
                  pSize(!((N - 1) % 8) ? (N - 1) >> 3 : 1 + ((N - 1) >> 3)),
                  bMask(std::vector<uint64_t>(pSize)),
                  pMask(std::vector<uint64_t>(N - 1)),
                  m_occurrences(SetWrapper<uintptr_t>{}),
                  bitMask(&bMask),
                  searchMask(&pMask),
                  occurrences(&m_occurrences)
            {
                generatePatternMask(this->pattern.cptr, this->patternLen, pMask);
                generateBMI2Mask(this->mask.cptr, this->maskLen, bMask);
            }

        private:
            template <PseudoContainer T>
            static void generateBMI2Mask(char *cptr, const size_t len, T &out)
            {
                uint64_t *optr = out.data();
                size_t cnt = 0;
                for (char *ptr = cptr; ptr < cptr + len; ++ptr, cnt+=4)
                {
                    *optr |= (!('?' ^ *ptr) ? 0xF : 0) << cnt;
                }
            }

            template <PseudoContainer T>
            static void generatePatternMask(char *cptr, const size_t len, T &out)
            {
                size_t cnt = 0;
                auto *mptr = reinterpret_cast<uint8_t*>(out.data());
                for (char *ptr = cptr; *ptr != '\xCC' &&
                     cnt < len; ++ptr, ++cnt)
                {
                    mptr[cnt] = 0xFF;
                }
            }
    };
}
#endif //PATTERN_HPP
