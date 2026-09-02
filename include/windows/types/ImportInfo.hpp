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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <unordered_map>

namespace PMO
{
    struct ImportInfo
    {
        HMODULE mod = nullptr;
        const char *name;
        std::unordered_map<uintptr_t, std::string> fnNames{};
        std::unordered_map<uintptr_t, uintptr_t> thunks{};

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
        return std::hash<std::string>{}(a.name);
    }
};
#endif //IMPORTINFO_HPP
