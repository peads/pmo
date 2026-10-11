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
                        // if (offsets)
                            // offsets->push_back(offset);
                    }
                }
                return result;
            }
        private:
            SetWrapper<uintptr_t> m_occurrences{};
            std::vector<char> m_pattern;
            std::vector<char> m_code;

            // template <typename T>
            // static T generateTypedMask(T n) noexcept requires (std::is_unsigned_v<T>)
            // {
            //     if (!n)
            //         return 0;
            //     int bits = std::bit_width(n);
            //     if (bits >= (sizeof(n) << 3))
            //     {
            //         return static_cast<T>(-1);
            //     }
            //     return std::move((static_cast<T>(1) << bits) - 1);
            // }

            template <typename T>
            static auto storeData(T data, const size_t len) noexcept
            {
                std::vector<char> result{};
                const size_t plen = (len + (STRIDE - 1ULL)) & ~(STRIDE - 1ULL);
                for (size_t i = 0; i < plen; ++i)
                {
                    if (i < len)
                        result.push_back(data[i]);
                    else
                        result.push_back(0);
                }
                return result;
            }

            template <typename U>
            static auto generateMask(U mask, const size_t len) noexcept
            {
                static constexpr auto EFFS = static_cast<char>(0xFF);
                std::vector<std::vector<char>> result(2);
                const size_t plen = (len + (STRIDE - 1ULL)) & ~(STRIDE - 1ULL);
                for (size_t i = 0; i < plen; ++i)
                {
                    if (i >= len)
                    {
                        result[0].push_back(0);
                        result[1].push_back(0);
                    }
                    else
                    {
                        if ('?' != mask[i])
                        {
                            result[0].push_back(EFFS);//generateTypedMask(static_cast<uint8_t>(pattern[i])));
                            result[1].push_back(0);
                        }
                        else
                        {
                            result[1].push_back(EFFS);
                            result[0].push_back(EFFS);
                        }
                    }
                }
                return result;
            }

        public:
            const std::string mask;
            const size_t patternLen;
            const size_t codeLen;
            std::array<PointerUnion, 2> masks{};

        private:
            std::vector<std::vector<char>> m_mask;

        public:
            template <size_t N, size_t M, size_t P>
            Pattern(const char (&pattern)[N], const char (&mask)[M], const char (&code)[P]) noexcept
                : m_pattern(storeData(pattern, N - 1)),
                  m_code(storeData(code, P - 1)),
                  mask(mask),
                  patternLen(N - 1),
                  codeLen(P - 1),
                  m_mask(generateMask(mask, M - 1))
            {
                static_assert(M == N);
                masks[0].str = m_mask[0].data();
                masks[1].str = m_mask[1].data();
            }

            template <typename T>
            Pattern(
                T pattern, const size_t plen, T mask, T code, const size_t clen
            ) noexcept
                : m_pattern(storeData(pattern, plen - 1)),
                  m_code(storeData(code, clen - 1)),
                  mask(mask),
                  patternLen(plen),
                  codeLen(clen),
                  m_mask(generateMask(mask, plen - 1))
            {
                masks[0].str = m_mask[0].data();
                masks[1].str = m_mask[1].data();
            }

            template <typename T>
            Pattern(T pattern,  const size_t plen, T code, const size_t clen) noexcept
                : m_pattern(storeData(pattern, plen - 1)),
                  m_code(storeData(code, clen - 1)),
                  mask(autoGenerateMask(pattern, plen)),
                  patternLen(plen),
                  codeLen(clen),
                  m_mask(generateMask(mask, plen - 1))
            {
                masks[0].str = m_mask[0].data();
                masks[1].str = m_mask[1].data();
            }

            [[nodiscard]] auto pattern() const noexcept
            {
                return PointerUnion{.str = m_pattern.data()};
            }

            [[nodiscard]] auto code() const noexcept
            {
                return PointerUnion{.str = m_code.data()};
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
                    // std::pair{
                    //     std::vector(this->m_mask, this->m_mask + this->patternLen),
                    //     std::vector(a.m_mask, a.m_mask + a.patternLen)
                    // },
                    std::pair{this->m_pattern, a.m_pattern},
                    std::pair{this->m_code, a.m_code}
                };
                return this->codeLen == a.codeLen
                    && this->patternLen == a.patternLen
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
