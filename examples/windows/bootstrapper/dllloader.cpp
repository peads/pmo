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
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <cstdint>
#include <iostream>
#include <windows.h>

typedef HRESULT (*hresultProducer)();
int main()
{
    const auto module = LoadLibrary("dwmapi.dll");
    if (!module)
        return -1;

    int result = 0;
    auto addr = GetProcAddress(module, "DwmFlush");
#ifdef IS_VERBOSE
    char dllPath[MAX_PATH];
    GetModuleFileName(module, dllPath, MAX_PATH);
    std::cout << "Bootstrapping dll: " << dllPath << std::endl;
#endif
    const auto DwmFlush = reinterpret_cast<hresultProducer>(addr);
    if (result += DwmFlush(); result != S_OK)
        std::cout << result << std::endl;
    else
        std::cout << "S_OK" << std::endl;

    addr = GetProcAddress(module, "DllCanUnloadNow");
    const auto DllCanUnloadNow = reinterpret_cast<hresultProducer>(addr);
    if (result += DllCanUnloadNow(); result != S_OK)
        std::cout << result << std::endl;
    else
        std::cout << "S_OK" << std::endl;

    result += !FreeLibrary(module);
    return result;
}
