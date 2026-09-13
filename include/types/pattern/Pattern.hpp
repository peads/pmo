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

namespace PMO
{
    template <typename T>
    inline T Pattern::generateTypedMask(T n) noexcept
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
    inline auto Pattern::generateBMI2Mask(const char (&smask)[M], const size_t len) noexcept
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

            auto mask = (((del - 0x0101'0101'0101'0101LLU) & (~del & 0x8080'8080'8080'8080LLU)) >>
                7);
            if (!isFine)
                mask *= 0xFF;
            else
            {
                mask |= (((del - 0x0101'0101'0101'0101LLU) & (~del & 0x8080'8080'8080'8080LLU)) >>
                    15);
                mask *= 0xF;
            }
            result.push_back(mask);
        }
        return std::move(result);
    }

    template <size_t M>
    inline auto Pattern::generatePatternMask(const char (&pattern)[M], const size_t len) noexcept
    {
        std::vector result(len, 0ULL);
        const auto optr = reinterpret_cast<uint8_t*>(result.data());
        for (size_t i = 0; i < M; ++i)
        {
            optr[i] |= generateTypedMask(pattern[i] & 0xFFULL);
        }
        return std::move(result);
    }

    template <size_t M>
    inline auto Pattern::autoGenerateMask(
        const char (&pattern)[M],
        std::vector<uint64_t> *offsets
    ) noexcept
    {
        PointerUnion ptr{.str = pattern};
        std::string result((M & 1 ? M - 1 : M) << 1, 'x');
        size_t width = 0;
        const auto pend = pattern + M - 1;
        for (char *idx = nullptr; ptr.cptr < pend;)
        {
            idx = ptr.cptr;
            if (const uint64_t val = startParseJmp(ptr.u8ptr, &width); !val)
                ptr.cptr = idx + 1; // reset on failure, move to the next
            else
            {
                // calculate mask offset
                const auto offset = static_cast<uintptr_t>(ptr.cptr - pattern) << 1;
                // move to next byte to process
                ptr.u8ptr += width;
                // calculate wildcard width
                width <<= 1;
                memcpy(result.data() + offset, std::string(width, '?').data(), width);
                if (offsets)
                    offsets->push_back(val);
            }
        }
        return std::move(result);
    }
}
#endif //HELPERS_HPP
