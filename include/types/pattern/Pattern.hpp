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

    inline std::vector<auint64_t> Pattern::generateByteMask(
        const char *smask,
        const size_t mlen
    )
        noexcept
    {
        std::vector<auint64_t> result{};
        if (!mlen)
            return result;
        const auto end = (mlen + 7ULL) & ~7ULL;
        size_t i = 0;
        std::array<uint8_t, 8> temp{};
        for (size_t k = 0; i < end; k = ++i % 8)
        {
            if (i < mlen)
                temp[k] = '?' == smask[i] ? 0xFF : 0;
            if (i && !k)
            {
                result.push_back({*reinterpret_cast<uint64_t*>(temp.data())});
                temp.fill(0);
            }
        }
        result.push_back({*reinterpret_cast<uint64_t*>(temp.data())});

        return result;
    }

    inline std::vector<auint64_t> Pattern::generatePatternMask(
        const char *pattern,
        const size_t plen
    ) noexcept
    {
        std::vector<auint64_t> result{};
        std::vector<uint8_t> temp{};
        const auto end = (plen + 7ULL) & ~7ULL;
        for (size_t i = 0; i < end; ++i)
        {
            const uint8_t c = pattern[i];
            if (i < plen)
                temp.push_back(c);
            else
                temp.push_back(0);
        }
        for (size_t i = 0; i < temp.size(); i += 8)
        {
            auint64_t e{generateTypedMask(*reinterpret_cast<uint64_t*>(&temp[i]))};
            result.push_back(e);
        }

        return result;
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
