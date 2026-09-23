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
#ifndef IMPORTINFO_HPP
#define IMPORTINFO_HPP

#include <iostream>
#include <windows.h>
#include <map>

namespace PMO
{
    struct ImportInfo
    {
        HMODULE mod = nullptr;
        std::string name{};
        std::map<WORD, std::tuple<uintptr_t, char*, bool>> exports{};
        std::map<uintptr_t, uintptr_t> thunks{};
        DWORD ordinalBase;

        bool operator<(const ImportInfo &a) const noexcept
        {
            return name < a.name;
        }

        bool operator==(const ImportInfo &a) const noexcept
        {
            return name == a.name;
        }
    };
}

template <>
struct std::hash<PMO::ImportInfo> {
    size_t operator()(const PMO::ImportInfo &a) const noexcept
    {
        // const std::size_t h1 = std::hash<std::string>{}(a.name);
        // const std::size_t h2 = std::hash<HMODULE>{}(a.mod);
        // return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        return std::hash<std::string>{}(a.name);
    }
};
#endif //IMPORTINFO_HPP
