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

#include <iostream>
#include <execution>
#include <map>

#include "windows/MemoryOps.hpp"

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define FORCE_INLINE_LAMBDA __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define FORCE_INLINE_LAMBDA [[msvc::forceinline]]
#else
    #define FORCE_INLINE_LAMBDA
#endif

static inline size_t printExports()
{
    // PMO::SetWrapper<std::string, std::set<std::string>> exports{};
    std::map<uintptr_t, std::string> exports{};
    const size_t cnt = PMO::traverseExports([&exports]<typename iter>(iter &it)FORCE_INLINE_LAMBDA
    {
        exports.insert(it->fnNames.begin(), it->fnNames.end());
        return it->fnNames.size();
    });

    std::cout <<
        std::reduce(std::execution::seq, exports.cbegin(), exports.cend(), std::string{},
             [](const std::string &acc, auto &keyVal)
             {
                 return acc + std::format("{} @ {:016X}\n", keyVal.second, keyVal.first);
             });
    return cnt;
}

extern "C" {
#ifndef BUILD_SHARED_LIB
    int main() noexcept
    {
#else
    __declspec(dllexport) int DllMain() noexcept
    {
#endif
        return !printExports();
    }
}
