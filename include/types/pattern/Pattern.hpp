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
#ifndef HELPERS_HPP
#define HELPERS_HPP
#include "types/pattern/PatternImpl.hpp"
#include "parse/ParseJmp.hpp"
#include <cstring>
#include <deque>

namespace PMO
{
    template <typename T>
    inline T Pattern::generateTypedMask(T n) requires (std::is_unsigned_v<T>)
    {
        if (!n)
            return 0;
        int bits = std::bit_width(n);
        if (bits >= (sizeof(n) << 3))
        {
            return static_cast<T>(-1);
        }
        return std::move((static_cast<T>(1) << bits) - 1);
    }

    inline std::vector<auint64_t> Pattern::generateBMI2Mask(
        const char *smask,
        const size_t mlen,
        const size_t len
    ) noexcept
    {
        const std::vector q(mlen, '?');
        const std::vector p(mlen, 'x');
        std::vector<auint64_t> result{};

        PointerUnion qs{.str = q.data()};
        PointerUnion xs{.str = p.data()};
        PointerUnion ptr{.str = smask};

        uint64_t prev = 0;
        for (auto i = 0ULL; i < len; ++i, ++ptr.u64ptr, ++qs.u64ptr, ++xs.u64ptr)
        {
            if (!(*ptr.u64ptr ^ *xs.u64ptr))
            {
                result.emplace_back(prev);
                prev = 0;
                continue;
            }

            const auto del = *ptr.u64ptr ^ *qs.u64ptr;
            auto mask = del - 0x0101'0101'0101'0101LLU;
            mask &= ~del & 0x8080'8080'8080'8080LLU;
            mask = (mask >> 7) * 0xFF;
            result.emplace_back(mask);
        }
        return result;
    }

    inline std::vector<auint64_t> Pattern::generatePatternMask(
        const char *pattern,
        const size_t plen
    ) noexcept
    {
        std::deque<auint64_t> result{};
        // const auto optr = reinterpret_cast<uint8_t*>(result.data());
        // for (size_t i = 0; i < plen; ++i)
        // {
        // optr[i] |= generateTypedMask(pattern[i] & 0xFFULL);
        // }
        // for (auto &e : result)
        // {
        //     auto *optr = reinterpret_cast<uint8_t *>(&e.i);
        //     for (size_t i = 0; i < 8; ++i)
        //     {
        //         optr[i] |= generateTypedMask(pattern[i] & 0xFFULL);
        //     }
        // }
        size_t i = 0;
        for (; i < plen; i += 8)
        {
            auint64_t e{*(uint64_t*)&pattern[i]};
            e.i = generateTypedMask(e.i);
            result.push_front(e);
        }
        // auto val = ((uint8_t*)&result.back().i);
        // for (size_t j = plen; j < i; ++j)
        // {
        //     val[j] = 0;
        // }

        return std::vector(result.begin(), result.end());
    }

    template <size_t M>
    inline std::string Pattern::autoGenerateMask(
        const char (&pattern)[M],
        std::vector<uint64_t> *offsets
    ) noexcept
    {
        PointerUnion ptr{.str = pattern};
        std::string result(M - 1, 'x');
        size_t width = 0;
        const auto pend = pattern + M - 1;
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
                if (offsets)
                    offsets->push_back(offset);
            }
        }
        return result;
    }

    template <size_t N>
    inline std::vector<auint64_t> Pattern::generateU64Vect(const char (&arr)[N]) noexcept
    {
        std::vector<auint64_t> result{};
        for (size_t i = 0; i < N; i += 8)
        {
            result.emplace_back(*static_cast<uint64_t*>((void*) (arr + i)));
        }
        return result;
    }
}
#endif //HELPERS_HPP
