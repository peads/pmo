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

#ifdef INCLUDE_SYSWHISPERS
#include "syscalls.h"
#endif

#ifndef _APISETLIBLOADER_
    extern "C" HMODULE LoadLibraryA(LPCSTR);
    // extern "C" HMODULE LoadLibraryW(LPCWSTR);
    // #ifdef UNICODE
    //     #define LoadLibrary  LoadLibraryW
    // #else
    //     #define LoadLibrary  LoadLibraryA
    // #endif // !UNICODE
    template <typename T>
    inline HMODULE LoadLibrary(std::basic_string<T> path)
    {
        return LoadLibraryA(path.string().c_str());
    }
#endif

#ifndef _SYSINFOAPI_H_
    extern "C" UINT GetSystemDirectoryA(LPSTR, UINT);
    extern "C" UINT GetSystemDirectoryW(LPWSTR, UINT);
    #ifdef UNICODE
        #define GetSystemDirectory  GetSystemDirectoryW
    #else
        #define GetSystemDirectory  GetSystemDirectoryA
    #endif // !UNICODE
#endif

#ifndef _MINWINDEF_
#define MAX_PATH 260
#endif

#define ARM64_ASM_HEADER ".section .text,\"xr\"\n.align 2\n.global "
#define ARM64_ASM_GLOBAL_1 ""
#define ARM64_ASM_GLOBAL_2 ","
#define ARM64_ASM_LINE_1 ":\n\tadrp x0, mapping\n\tldr x0, [x0, #:lo12:mapping + "
#define ARM64_ASM_LINE_2 "]\n\tbr x0\n"
#define ARM64_ASM_RDATA_1 ".section .rdata,\"dr\"\n.align 4\n\tdllName:\n\t.asciz \""
#define ARM64_ASM_RDATA_2 "\"\n"

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define X64_ASM_HEADER ".intel_syntax noprefix\n.section .text\n.global "
#define X64_ASM_GLOBAL_1 ""
#define X64_ASM_GLOBAL_2 ","
#define X64_ASM_LINE_1 ":\n\tmov rax, [rip + mapping]\n\tjmp [rax + "
#define X64_ASM_LINE_2 "]\n"
#define X64_ASM_RDATA_1 ".section .rdata\n\tdllName: .asciz \""
#define X64_ASM_RDATA_2 "\"\n"
// #define ASM_EXTENSION ".s"
#else
#define X64_ASM_HEADER "bits 64\nsection .text\nextern mapping\nglobal "
#define X64_ASM_GLOBAL_1 ""
#define X64_ASM_GLOBAL_2 ","
#define X64_ASM_LINE_1 ":\n\tmov rax, [rel mapping]\n\tjmp [rax + "
#define X64_ASM_LINE_2 "]\n"
#define X64_ASM_RDATA_1 "section .rdata\n\tdllName db \""
#define X64_ASM_RDATA_2 "\", 0\n"
// #define ASM_EXTENSION ".asm"
#endif

#ifndef _NTDEF_
typedef struct $UNICODE_STRING {
    uint16_t Length;
    uint16_t MaximumLength;
    wchar_t *Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef int32_t NTSTATUS, *PNTSTATUS;
#endif

static inline struct DllInfo
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

// static HMODULE loadLibrary(const std::filesystem::path &path)
// {
//     static const HMODULE ntDll = PMO::getModule(NTDLL);
//     if (!ntDll)
//         return nullptr;
//
//     std::map<WORD, std::tuple<uintptr_t, char*, bool>> exports;
//     PMO::findExports(ntDll, exports);
//
//     auto view = PMO::findProcByName("LdrLoadDll", exports);
//     DllQuadFunction LdrLoadDll = nullptr;
//     if (view.empty() || !((LdrLoadDll = reinterpret_cast<DllQuadFunction>(view.back()))))
//         return nullptr;
//
//     HANDLE module = nullptr;
//     UNICODE_STRING uname;
//     const auto wname = path.wstring();
//     const size_t len = path.wstring().length();
//
//     uname.Length = static_cast<USHORT>(len * sizeof(wchar_t));
//     uname.MaximumLength = static_cast<USHORT>((len + 1) * sizeof(wchar_t));
//     uname.Buffer = const_cast<wchar_t*>(wname.c_str());
//
//     if (LdrLoadDll(nullptr, 0, &uname, &module) >= 0L)
//     {
//         return static_cast<HMODULE>(module);
//     }
//     return nullptr;
// }

static bool populateDllInfo(const std::filesystem::path &name)
{
    dllInfo.module = LoadLibrary(name.string().c_str());
    if (!dllInfo.module)
        return false;
    return (dllInfo.ordinalBase = PMO::findExports(dllInfo.module, dllInfo.exports));
}
#endif //EXAMPLE4_BOOTSTRAPPER_HPP
