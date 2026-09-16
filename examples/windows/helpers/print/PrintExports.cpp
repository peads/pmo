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

#define DWMAPI_PATH "C:/Windows/System32/dwmapi.dll"

extern "C" {
#ifndef BUILD_SHARED_LIB
    int main() noexcept
    {
#else
    __declspec(dllexport) int DllMain() noexcept
    {
#endif
        const HMODULE module = LoadLibrary(DWMAPI_PATH);
        std::map<uintptr_t, std::pair<WORD, std::string>> exports{};
        const DWORD ordBase = PMO::findExports(module, exports);
        std::stringstream buf;

        for (const auto &[fn, tup] : exports)
        {
            const auto &[ord, name] = tup;
            buf << std::format("{:016X}: {:03}:{:03} -> {}\n", fn, ord, ordBase + ord, name);
        }

        const auto &addr = (exports | std::views::filter([](auto &e)
        {
            auto &[ord, name] = e.second;
            return name == "DwmFlush";
        }) | std::views::keys).back();

        buf << "DwmFlush result: ";
        HRESULT (*DwmFlush)() = *reinterpret_cast<HRESULT (*)()>(addr);
        const HRESULT result = DwmFlush();

        if (result != S_OK)
            buf << result << std::endl;
        else
            buf << "S_OK" << std::endl;
        std::cout << buf.str();

        FreeLibrary(module);
        return result;
    }
}
