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
#include "parse/ParseJmp.hpp"
#include "types/PointerUnion.hpp"
#include "types/SetWrapper.hpp"

#ifndef STRIDE
#undef STRIDE
#define STRIDE 8ULL
#endif

namespace PMO
{
    class Pattern
    {
        public:
            template <typename T>
            static std::string autoGenerateMask(
                T pattern, const size_t len
            ) noexcept
            {
                PointerUnion ptr{.str = pattern};
                std::string result(len - 1, 'x');
                size_t width = 0;
                const auto pend = pattern + len - 1;
                for (char *idx = nullptr; ptr.cptr < pend;)
                {
                    idx = ptr.cptr;
                    if (const uint64_t offset = startParseJmp(ptr.u8ptr, &width); !offset)
                        ptr.cptr = idx + 1; // reset on failure, move to the next
                    else
                    {
                        // calculate mask offset
                        const auto moffset = static_cast<uintptr_t>(ptr.cptr - pattern);
                        // move to next byte to process
                        ptr.u8ptr += width;
                        memcpy(result.data() + moffset, std::string(width, '?').data(), width);
                    }
                }
                return result;
            }
        private:
            SetWrapper<uintptr_t> m_occurrences{};
            const char *const m_pattern;
            const char *const m_code;

            static inline std::vector<uint64_t> generatePatternMask(size_t n) {
                if (n <= 0)
                    return {};
                std::vector<uint64_t> result{};

                for (; n >= 8; n -= 8)
                    result.push_back(0xFFFFFFFFFFFFFFFFULL);

                if (n)
#ifdef IS_LITTLE_ENDIAN
                    result.push_back((0xFFFFFFFFFFFFFFFFULL >> (64 - (n << 3))));
#else
                    result.push_back((0xFFFFFFFFFFFFFFFFULL << (64 - (n << 3))));
#endif
                return result;
            }

            // template <typename T>
            // static auto storeData(T data, const size_t len) noexcept
            // {
            //     std::vector<char> result{};
            //     const size_t plen = (len + (STRIDE - 1ULL)) & ~(STRIDE - 1ULL);
            //     for (size_t i = 0; i < plen; ++i)
            //     {
            //         result.push_back(i < len ? data[i] : 0);
            //     }
            //     return result;
            // }

            template <typename U>
            static auto generateByteMask(U &mask, const size_t len) noexcept
            {
                static constexpr auto EFFS = static_cast<char>(0xFF);
                const size_t plen = (len + (STRIDE - 1ULL)) & ~(STRIDE - 1ULL);
                std::vector<char> result(plen);
                for (size_t i = 0; i < plen; ++i)
                {
                    result[i] = (i >= len || '?' != mask[i]) ? 0 : EFFS;
                }
                return result;
            }

        public:
            const std::string mask;
            const size_t patternLen;
            const size_t codeLen;

        private:
            const std::vector<char> m_byteMask;
            std::vector<uint64_t> m_patternMask;

        public:
            const size_t pSize;
            template <size_t N, size_t M, size_t P>
            Pattern(const char (&pattern)[N], const char (&mask)[M], const char (&code)[P]) noexcept
                : m_pattern(pattern),
                  m_code(code),
                  mask(mask),
                  patternLen(N - 1),
                  codeLen(P - 1),
                  m_byteMask(generateByteMask(mask, M - 1)),
                  m_patternMask(generatePatternMask(N - 1)),
                  pSize(m_patternMask.size())
            {
                static_assert(M == N);
            }

            // template <typename T>
            // Pattern(T pattern,  const size_t plen, T code, const size_t clen) noexcept
            //     : m_pattern(storeData(pattern, plen - 1)),
            //       m_code(storeData(code, clen - 1)),
            //       mask(autoGenerateMask(pattern, plen)),
            //       patternLen(plen),
            //       codeLen(clen),
            //       m_byteMask(generateByteMask(mask, plen - 1)),
            //       m_patternMask(generatePatternMask(plen - 1)),
            //       pSize(m_patternMask.size())
            // {}

            [[nodiscard]] auto byteMask() const noexcept
            {
                return PointerUnion{.str = m_byteMask.data()};
            }

            [[nodiscard]] auto patternMask() noexcept
            {
                return PointerUnion{.u64ptr = m_patternMask.data()};
            }

            [[nodiscard]] auto pattern() const noexcept
            {
                return PointerUnion{.str = m_pattern};
            }

            [[nodiscard]] auto code() const noexcept
            {
                return PointerUnion{.str = m_code};
            }

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
            bool operator==(const Pattern &a) const noexcept
            {
                auto arr = {
                    std::pair{
                        std::vector(this->m_pattern, this->m_pattern + this->patternLen),
                        std::vector(a.m_pattern, a.m_pattern + a.patternLen)
                    },
                    std::pair{
                        std::vector(this->m_code, this->m_code + this->codeLen),
                        std::vector(a.m_code, a.m_code + a.codeLen)
                    }
                };
                return this->codeLen == a.codeLen
                    && this->patternLen == a.patternLen
                    && this->mask == a.mask
                    && std::ranges::all_of(arr,
                    [](auto &e)
                    {
                        auto &[x,y] = e;
                        return std::ranges::equal(x, y);
                    });
            }
    };
}
#endif // PATTERN_HPP
