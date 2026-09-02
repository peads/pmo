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
#ifndef BYTEQUEUE_HPP
#define BYTEQUEUE_HPP
#include <streambuf>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace PMO
{
#if !defined(__cplusplus) || __cplusplus < 201103L
#if defined(__clang__) || defined(__GNUC__)
#define alignas(x) __attribute__((aligned(x)))
#elif defined(_MSC_VER)
#define alignas(x) __declspec(align(x))
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#include <stdalign.h>
#else
#error "No alignment mechanism found."
#endif
#endif

    class ByteQueue final : public std::streambuf
    {
        protected:

        /**
        * Write to the ostream.
        */
        int_type overflow(const int_type ch) override
        {
            if (ch != traits_type::eof())
            {
                std::lock_guard<std::mutex> lock(mtx_);
                queue_.push(traits_type::to_char_type(ch));
                cv_.notify_one();
            }
            return ch;
        }

        /**
        * Read from the istream.
        */
        int_type underflow() override
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock,
                     [this]()
                     {
                         return !queue_.empty() || closed_; // Await data, or closed
                     });

            if (queue_.empty() && closed_)
            {
                return traits_type::eof();
            }

            // Return peek of first character. sbumpc() consumes
            return traits_type::to_int_type(queue_.front());
        }

        /**
        * Consume character
        */
        int_type uflow() override
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock,
                     [this]()
                     {
                         return !queue_.empty() || closed_; // Await data, or closed
                     });

            if (queue_.empty() && closed_)
            {
                return traits_type::eof();
            }

            char ch = queue_.front();
            queue_.pop();
            return traits_type::to_int_type(ch);
        }

        public:
        /**
        * YEET
        */
        void close()
        {
            std::lock_guard<std::mutex> lock(mtx_);
            closed_ = true;
            cv_.notify_all(); // Push EOF
        }

        private:
        std::queue<char> queue_;
        std::mutex mtx_;
        std::condition_variable cv_;
        bool closed_ = false;
    };
}
#endif //BYTEQUEUE_HPP
