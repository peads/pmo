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
#include <vector>
#include <ranges>

namespace PMO
{
    class Pattern final
    {
        template <typename T>
        static inline T generateTypedMask(T n) noexcept
        {
            if (!n)
                return 0;
            int bits = std::bit_width(n);
            if (bits >= (sizeof(T) << 3))
            {
                return static_cast<T>(~static_cast<T>(0));
            }
            return (static_cast<T>(1) << bits) - 1;
        }

        /**
         *
         * @param cptr NULL-terminated pattern mask string
         * @param optr Output pointer
         * @param isFine Indicates coarseness of mask
         */
        static inline void generateBMI2Mask(char *const cptr, uint64_t *optr, const bool isFine = false) noexcept
        {
            size_t cnt = 0;
            uint8_t incre = 8;
            uint8_t maskByte = 0xFFu;
            if (isFine)
            {
                incre = 4;
                maskByte = 0xFu;
            }

            for (char *ptr = cptr; *ptr; ++ptr, cnt += incre)
            {
                *optr |= (!('?' ^ *ptr) ? maskByte : 0u) << cnt;
                if (cnt > 0 && !(cnt & 0x3F))
                {
                    ++optr;
                    cnt = 0;
                }
            }
        }

        static inline void generatePatternMask(uint64_t *const &in, const size_t len, uint64_t *optr) noexcept
        {
            const uint64_t *iptr = in;
            for (size_t shift = 0, cnt = 0; cnt < len; ++cnt, shift += 8)
            {
                if (cnt > 0 && !(cnt & 0x7ULL))
                {
                    shift = 0;
                    ++optr;
                    ++iptr;
                }
                *optr |= generateTypedMask((*iptr >> shift) & 0xFFULL) << shift;
            }
        }

        public:
            const size_t patternLen;
            const size_t maskLen;
            const size_t codeLen;
            const size_t pSize;

        private:
            const char *const m_pattern;
            const char *const m_mask;
            const char *const m_code;
            SetWrapper<uintptr_t> m_occurrences{};
            std::vector<uint64_t> byteMask;
            std::vector<uint64_t> searchMask;

        public:
            const PointerUnion pattern;
            const PointerUnion mask;
            const PointerUnion code;
            // const SetWrapper<uint64_t> &occurrences = m_occurrences;

            void reset() noexcept
            {
                m_occurrences.clear();
            }

            [[nodiscard]] size_t size() const noexcept
            {
                return m_occurrences.size();
            }

            [[nodiscard]] auto begin() const noexcept
            {
                return m_occurrences.begin();
            }

            [[nodiscard]] auto end() const noexcept
            {
                return m_occurrences.end();
            }

            [[nodiscard]] bool empty() const noexcept
            {
                return m_occurrences.empty();
            }

            void push_back(const uint64_t in) noexcept
            {
                m_occurrences.push_back(in);
            }

            [[nodiscard]] PointerUnion pop_back() noexcept
            {
                const auto result = back();
                m_occurrences.pop_back();
                return result;
            }

            [[nodiscard]] PointerUnion back() noexcept
            {
                return PointerUnion{.address = m_occurrences.back()};
            }

            [[nodiscard]] std::ranges::zip_view<
                std::span<uint64_t>,
                std::ranges::ref_view<const std::vector<uint64_t>>,
                std::ranges::ref_view<const std::vector<uint64_t>>> view() const noexcept
            {
                return std::views::zip(std::span(pattern.u64ptr, pSize), searchMask, byteMask);
            }

            template <size_t N, size_t M, size_t P>
            Pattern(const char (&pattern)[N], const char (&mask)[M], const char (&code)[P])
                : patternLen(N - 1),
                  maskLen(M),
                  codeLen(P - 1),
                  pSize((N & 1 ? N + 1 : N) >> 3),
                  m_pattern(pattern),
                  m_mask(mask),
                  m_code(code),
                  byteMask(std::vector<uint64_t>(pSize, 0)),
                  searchMask(std::vector<uint64_t>(pSize, 0)),
                  pattern{.str = m_pattern},
                  mask{.str = m_mask},
                  code{.str = m_code}
            {
                generatePatternMask(this->pattern.u64ptr, N - 1, searchMask.data());
                generateBMI2Mask(this->mask.cptr, byteMask.data(), N - 1 < M - 1);
            }
    };
}
#endif //PATTERN_HPP
