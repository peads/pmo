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

#include "windows/MemoryOps.hpp"

extern "C" {
#ifndef BUILD_SHARED_LIB
    int main() noexcept
    {
#else
    __declspec(dllexport) int DllMain() noexcept
    {
#endif
        PMO::SetWrapper<PMO::ImportInfo> imports{};
        findImports(GetModuleHandle(nullptr), imports);
        PMO::ImportInfo::printExports(imports);
        return !imports.empty();
    }
}
