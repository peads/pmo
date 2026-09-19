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
#include <fstream>

inline struct DllInfo
{
    HMODULE module{};
    DWORD ordinalBase{};
    std::map<WORD, std::tuple<uintptr_t, char*, bool>> exports{};
} dllInfo;

typedef void (*UnicodeBiConsumer)(UNICODE_STRING *, const wchar_t *);
typedef long (*DllQuadFunction)(const wchar_t *, ULONG, UNICODE_STRING *, void *);

static HMODULE loadLibrary(const char *name)
{
    static const HMODULE ntDll = GetModuleHandle("ntdll.dll");
    if (!ntDll)
        return nullptr;

    static const auto RtlInitUnicodeString = reinterpret_cast<UnicodeBiConsumer>(
        GetProcAddress(ntDll, "RtlInitUnicodeString"));
    static const auto LdrLoadDll = reinterpret_cast<DllQuadFunction>(
        GetProcAddress(ntDll, "LdrLoadDll"));

    if (!(RtlInitUnicodeString && LdrLoadDll))
        return nullptr;

    const std::string sName(name);
    const std::wstring wsName(sName.begin(), sName.end());

    HANDLE module = nullptr;
    UNICODE_STRING uname;
    RtlInitUnicodeString(&uname, wsName.c_str());

    if (LdrLoadDll(nullptr, 0, &uname, &module) >= 0) // NT_SUCCESS(status)
    {
        return static_cast<HMODULE>(module);
    }
    return nullptr;
}

static bool populateDllInfo(const std::filesystem::path &name)
{
    dllInfo.module = loadLibrary(name.string().c_str());
    dllInfo.ordinalBase = PMO::findExports(dllInfo.module, dllInfo.exports);
    return !!dllInfo.module;
}

static std::filesystem::path generateDllPath(const char *const name)
{
    std::filesystem::path path(name);
    if (path.is_relative())
    {
        char systemDir[MAX_PATH];
        GetSystemDirectory(systemDir, MAX_PATH);
        path = std::filesystem::path(systemDir);
        path.append(name);
    }
    return std::move(path);
}
#endif //EXAMPLE4_BOOTSTRAPPER_HPP
