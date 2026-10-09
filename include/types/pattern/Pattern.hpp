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

    inline std::vector<uint8_t> Pattern::generateByteMask(const char *smask,const size_t mlen)noexcept
    {
        std::vector<uint8_t> result{};
        if (!mlen)
            return result;

        const auto end = (mlen + 7ULL) & ~7ULL;
        for (size_t i = 0; i < end; ++i)
        {
            if (i < mlen)
                result.push_back('?' == smask[i] ? 0xFF : 0);
            else
                result.push_back(0);
        }
    
        return result;
    }

    inline std::vector<uint8_t> Pattern::generatePatternMask(
        const char *pattern,
        const size_t plen
    ) noexcept
    {
        std::vector<uint8_t> result{};
        const auto end = (plen + 7ULL) & ~7ULL;
        for (size_t i = 0; i < end; ++i)
        {
            const uint8_t c = pattern[i];
            if (i < plen)
                result.push_back(generateTypedMask(c));
            else
                result.push_back(0);
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
}
#endif //HELPERS_HPP
