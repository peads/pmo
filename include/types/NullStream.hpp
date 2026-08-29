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
#ifndef NULLSTREAM_HPP
#define NULLSTREAM_HPP
#include <iostream>
namespace PMO
{
    class NullStream final : public std::ostream
    {
        class NullStreamBuffer final : public std::streambuf
        {
            protected:
            int overflow(const int c) override
            {
                return c;
            }
        };

        NullStreamBuffer buf;

        NullStream() : std::ostream(&buf){}
        ~NullStream() override = default;

        public:
        NullStream(const NullStream &) = delete;
        NullStream &operator=(const NullStream &) = delete;
        NullStream(NullStream &&) = delete;
        NullStream &operator=(NullStream &&) = delete;

        static NullStream &getInstance()
        {
            static NullStream instance;
            return instance;
        }
    };
    static NullStream &devnull = NullStream::getInstance();
}
#endif //NULLSTREAM_HPP
