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
         * @param smask string mask to convert
         * @param len
         */
        template <size_t M>
        static inline auto generateBMI2Mask(const char (&smask)[M], const size_t len) noexcept
        {
            const bool isFine = (M >> 3) > len;
            const std::vector q(M, '?');
            const std::vector p(M, 'x');
            std::vector<uint64_t> result{};

            PointerUnion qs{.str = q.data()};
            PointerUnion xs{.str = p.data()};
            PointerUnion ptr{.str = smask};
            // auto *qs = reinterpret_cast<const uint64_t*>(q.data()),
            //      *xs = reinterpret_cast<const uint64_t*>(p.data()),
            //      *ptr = reinterpret_cast<const uint64_t*>(smask);
            uint64_t prev = 0;
            for (auto i = 0ULL; i < len; ++i, ++ptr.u64ptr, ++qs.u64ptr, ++xs.u64ptr)
            {
                if (!(*ptr.u64ptr ^ *xs.u64ptr))
                {
                    result.push_back(prev);
                    prev = 0;
                    continue;
                }

                const auto del = *ptr.u64ptr ^ *qs.u64ptr;
                // auto mask = del - 0x0101'0101'0101'0101LLU;
                // mask &= ~del & 0x8080'8080'8080'8080LLU;
                // mask = (mask >> 15) * 0xF;

                auto mask = (((del - 0x0101'0101'0101'0101LLU) & (~del & 0x8080'8080'8080'8080LLU)) >> 7);
                if (!isFine)
                    mask *= 0xFF;
                else
                {
                    mask |= (((del - 0x0101'0101'0101'0101LLU) & (~del & 0x8080'8080'8080'8080LLU)) >> 15);
                    mask *= 0xF;
                }
                result.push_back(mask);
            }
            return std::move(result);
        }

        template <size_t M>
        static inline auto generatePatternMask(const char (&pattern)[M], const size_t len) noexcept
        {
            std::vector result(len, 0ULL);
            const auto optr = reinterpret_cast<uint8_t*>(result.data());
            for (size_t i = 0; i < M; ++i)
            {
                optr[i] |= generateTypedMask(pattern[i] & 0xFFULL);
            }
            return std::move(result);
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

            [[nodiscard]] auto pmsk() const noexcept
            {
                return std::ranges::ref_view(searchMask) | std::views::as_const;
            }


            [[nodiscard]] auto bmsk() const noexcept
            {
                return std::ranges::ref_view(byteMask) | std::views::as_const;
            }

            template <size_t N, size_t M, size_t P>
            Pattern(const char (&pattern)[N], const char (&mask)[M], const char (&code)[P])
                : patternLen(N - 1ULL),
                  maskLen(M),
                  codeLen(P - 1ULL),
                  pSize((N & 1ULL ? N + 1ULL : N) >> 3), // ceil(N) / 8
                  m_pattern(pattern),
                  m_mask(mask),
                  m_code(code),
                  byteMask(generateBMI2Mask(mask, pSize)),//N < M ? pSize << 1: pSize)),
                  searchMask(generatePatternMask(pattern, pSize)),
                  pattern{.str = m_pattern},
                  mask{.str = m_mask},
                  code{.str = m_code}
            {
                static_assert(((N - 1ULL) == (M - 1ULL)) || (((N - 1ULL)) == (M - 1ULL) >> 1),
                    "Pattern size to mask size ratio must be 1:1, or 2:1.");
            }

            Pattern() = delete;
            Pattern(const Pattern&) = delete;
            Pattern& operator=(const Pattern&) = delete;
            Pattern(const Pattern&&) = delete;
            Pattern& operator=(const Pattern&&) = delete;
    };
}
#endif //PATTERN_HPP
