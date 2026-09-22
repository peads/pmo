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
#ifndef EXAMPLE4_BOOTSTRAPPER_HPP
#define EXAMPLE4_BOOTSTRAPPER_HPP
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows/MemoryOps.hpp>
#include "syscalls.h"

inline struct DllInfo
{
    HMODULE module{};
    DWORD ordinalBase{};
    std::map<WORD, std::tuple<uintptr_t, char*, bool>> exports{};
} dllInfo;

typedef void (*UnicodeBiConsumer)(UNICODE_STRING *, const wchar_t *);
typedef NTSTATUS (*DllQuadFunction)(UNICODE_STRING *, ULONG, UNICODE_STRING *, void *);

static void generateDllPath(std::filesystem::path &path)
{
    if (path.is_relative())
    {
        char systemDir[MAX_PATH];
        GetSystemDirectory(systemDir, MAX_PATH);
        path = std::filesystem::path(systemDir).append(path.filename().string());
    }
}

static HMODULE loadLibrary(const std::filesystem::path &path)
{
    static const HMODULE ntDll = PMO::getModule(NTDLL);
    if (!ntDll)
        return nullptr;

    std::map<WORD, std::tuple<uintptr_t, char*, bool>> exports;
    PMO::findExports(ntDll, exports);

    auto view = PMO::findProcByName("LdrLoadDll", exports);
    DllQuadFunction LdrLoadDll = nullptr;
    if (view.empty() || !((LdrLoadDll = reinterpret_cast<DllQuadFunction>(view.back()))))
        return nullptr;

    HANDLE module = nullptr;
    UNICODE_STRING uname;
    const auto wname = path.wstring();
    const size_t len = path.wstring().length();

    uname.Length = static_cast<USHORT>(len * sizeof(wchar_t));
    uname.MaximumLength = static_cast<USHORT>((len + 1) * sizeof(wchar_t));
    uname.Buffer = const_cast<wchar_t*>(wname.c_str());

    if (LdrLoadDll(nullptr, 0, &uname, &module) >= 0L)
    {
        return static_cast<HMODULE>(module);
    }
    return nullptr;
}

static bool populateDllInfo(const std::filesystem::path &name)
{
    dllInfo.module = loadLibrary(name);
    if (!dllInfo.module)
        return false;
    return (dllInfo.ordinalBase = PMO::findExports(dllInfo.module, dllInfo.exports));
}
#endif //EXAMPLE4_BOOTSTRAPPER_HPP
