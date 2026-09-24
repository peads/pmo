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

static inline auto searchAddr(const char *key, std::map<WORD, std::tuple<uintptr_t, char*, bool>> &exports) noexcept
{
    // ReSharper disable once CppLocalVariableMayBeConst
    auto view = exports | std::views::filter([&key](auto &e)
    {
        auto &[fn, name, isNamed] = e.second;
        return isNamed && !strcmp(name, key);
    }) | std::views::values | std::views::keys;
    if (!exports.empty())
        return view.back();
    return 0ULL;
}

typedef HRESULT (*hresultProducer)();

int main(int argc, const char **argv) noexcept
{
    HMODULE module = nullptr;
    if (argc > 1)
        module = LoadLibrary(argv[1]);
    else
        module = LoadLibrary(DWMAPI_NAME);

    const std::filesystem::path path{PMO::getModuleFullPath(module)};

    std::map<WORD, std::tuple<uintptr_t, char*, bool>> exports{};
    const DWORD ordBase = PMO::findExports(module, exports);
    std::stringstream buf{};

    buf << path.string() << std::endl;
    for (const auto &[ord, tup] : exports)
    {
        const auto &[fn, name, isNamed] = tup;
        buf << std::format("{:016X}: {:03} -> {}\n", fn, ordBase + ord, isNamed ? name : "");
    }

    if (auto addr = searchAddr("DwmFlush", exports))
    {
        buf << "DwmFlush result: ";
        const hresultProducer DwmFlush = *reinterpret_cast<hresultProducer>(addr);
        HRESULT result = DwmFlush();

        if (result != S_OK)
            buf << result << std::endl;
        else
            buf << "S_OK" << std::endl;

        addr = searchAddr("DwmFlush", exports);
        buf << "DllCanUnloadNow result: ";
        const hresultProducer DllCanUnloadNow = *reinterpret_cast<hresultProducer>(addr);
        result = DllCanUnloadNow();
        if (result != S_OK)
            buf << result << std::endl;
        else
            buf << "S_OK" << std::endl;
    }
    std::cout << buf.str();

    FreeLibrary(module);
    return 0;
}
