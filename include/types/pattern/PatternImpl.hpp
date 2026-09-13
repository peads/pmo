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

#include "types/PointerUnion.hpp"
#include "types/SetWrapper.hpp"
#include <vector>
#include <ranges>

namespace PMO
{
    class Pattern final
    {
        template <size_t M>
        static inline auto generatePatternMask(const char (&)[M], size_t) noexcept;
        template <size_t M>
        static inline auto generateBMI2Mask(const char (&)[M], size_t) noexcept;
        template <typename T>
        static inline T generateTypedMask(T) noexcept;

        public:
            template <size_t M>
            static inline auto autoGenerateMask(const char (&)[M], std::vector<uint64_t> * = nullptr) noexcept;

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
                  pSize((N & 1ULL ? N + 1ULL : N) >> 3),
                  m_pattern(pattern),
                  m_mask(mask),
                  m_code(code),
                  byteMask(generateBMI2Mask(mask, pSize)),
                  searchMask(generatePatternMask(pattern, pSize)),
                  pattern{.str = m_pattern},
                  mask{.str = m_mask},
                  code{.str = m_code}
            {
                static_assert(N == M, "Pattern size to mask size ratio must be 1:1.");
            }

            Pattern() = delete;
            Pattern(const Pattern&) = delete;
            Pattern& operator=(const Pattern&) = delete;
            Pattern(const Pattern&&) = delete;
            Pattern& operator=(const Pattern&&) = delete;
    };
}
#endif //PATTERN_HPP
