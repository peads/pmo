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

#define DWMAPI_NAME "dwmapi.dll"
static inline auto searchAddr(const char *key, std::map<WORD, std::tuple<uintptr_t, char*, bool>> &exports)
{
    return (exports | std::views::filter([&key](auto &e)
    {
        auto &[fn, name, isNamed] = e.second;
        return isNamed && !strcmp(name, key);
    }) | std::views::values | std::views::keys).back();
}
typedef HRESULT (*hresultProducer)();

extern "C" {
#ifndef BUILD_SHARED_LIB
    int main() noexcept
    {
#else
    __declspec(dllexport) int DllMain() noexcept
    {
#endif
        char systemDir[MAX_PATH];
        GetSystemDirectory(systemDir, MAX_PATH);
        std::filesystem::path path(systemDir);
        path.append(DWMAPI_NAME);
        const HMODULE module = LoadLibrary(path.string().c_str());

        std::map<WORD, std::tuple<uintptr_t, char*, bool>> exports{};
        const DWORD ordBase = PMO::findExports(module, exports);
        std::stringstream buf;

        for (const auto &[ord, tup] : exports)
        {
            const auto &[fn, name, isNamed] = tup;
            buf << std::format("{:016X}: {:03}:{:03} -> {}\n", fn, ord, ordBase + ord, isNamed ? name : "");
        }

        auto addr = searchAddr("DwmFlush", exports);

        buf << "DwmFlush result: ";
        // HRESULT (*DwmFlush)()
        const hresultProducer DwmFlush = *reinterpret_cast<hresultProducer>(addr);
        HRESULT result = DwmFlush();

        if (result != S_OK)
            buf << result << std::endl;
        else
            buf << "S_OK" << std::endl;

        addr = searchAddr("DwmFlush", exports);

        buf << "DllCanUnloadNow result: ";
        // HRESULT (*DwmFlush)()
        const hresultProducer DllCanUnloadNow = *reinterpret_cast<hresultProducer>(addr);
        result = DllCanUnloadNow();
        if (result != S_OK)
            buf << result << std::endl;
        else
            buf << "S_OK" << std::endl;

        std::cout << buf.str();

        FreeLibrary(module);
        return result;
    }
}
