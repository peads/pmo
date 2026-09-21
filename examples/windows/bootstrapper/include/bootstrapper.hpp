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
#define NTDLL L"\x6E\x74\x64\x6C\x6C\x2E\x64\x6C\x6C"
#ifdef _WIN64
#define GET_PEB __readgsqword(0x60)
#else
#define GET_PEB __readfsdword(0x30)
#endif
#define GET_FROM_OFFSET_PEB(off) *reinterpret_cast<void**>(GET_PEB + off)

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

static HMODULE getCurrentModule()
{
    return static_cast<HMODULE>(GET_FROM_OFFSET_PEB(16));
}

template <size_t N>
static HMODULE getModule(const wchar_t (&wname)[N])
{
    void *ldr = GET_FROM_OFFSET_PEB(24);
    const SW3_LDR_DATA_TABLE_ENTRY *pld = static_cast<SW3_LDR_DATA_TABLE_ENTRY*>(ldr);

    // ReSharper disable once CppCStyleCast
    for (void **curr = (void**) &pld->InMemoryOrderLinks,
              **cend = curr;
         *curr != cend;
         curr = static_cast<void**>(*curr))
    {
        void *base = *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(curr) + 48); // curr->DllBase

        if (const UNICODE_STRING name = *reinterpret_cast<UNICODE_STRING*>(reinterpret_cast<
            uintptr_t>(curr) + 88); !lstrcmpW(name.Buffer, wname)) // curr->BaseDllName == wname
        {
            return static_cast<HMODULE>(base);
        }
    }
    return nullptr;
}

static HMODULE loadLibrary(const std::filesystem::path &path)
{
    static const HMODULE ntDll = getModule(NTDLL);
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
