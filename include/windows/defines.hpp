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
#ifndef PMO_WINDOWS_DEFINES_HPP
#define PMO_WINDOWS_DEFINES_HPP

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifndef _MSC_VER
    #define mbstowcs_s(len, dest, words, src, count) mbstowcs(dest, src, count)
    #define wcstombs_s(len, dest, src, count) wcstombs(dest, src, count)
#endif

#define NTDLL L"\x6E\x74\x64\x6C\x6C\x2E\x64\x6C\x6C"
#ifdef _WIN64
    #if defined(_M_ARM64) || defined(__aarch64__)
        #define CURRENT_PEB __readx18qword(0x60)
    #else
        #define CURRENT_PEB __readgsqword(0x60)
    #endif
#else
    #if defined(_M_ARM) || defined(__arm__)
        #define CURRENT_PEB __readx18dword(0x30)
    #else
        #define CURRENT_PEB __readfsdword(0x30)
    #endif
#endif

#define PEB_THIS_MODULE_OFFSET 0x10
#define PEB_LDR_OFFSET 0x18
#define GET_FROM_OFFSET_PEB(off) *reinterpret_cast<void**>(CURRENT_PEB + off)
#define CURRENT_MODULE static_cast<HMODULE>(GET_FROM_OFFSET_PEB(PEB_THIS_MODULE_OFFSET))
#define LDR_LIST_OFFSET 0x2
#define LDR_DLL_BASE_OFFSET 0x6
#define LDR_DLL_FULL_PATH_OFFSET 0x9
#define LDR_DLL_BASE_NAME_OFFSET 0xB

#ifndef PSAPI_VERSION
    typedef struct $MODULEINFO {
        LPVOID lpBaseOfDll;
        DWORD SizeOfImage;
        LPVOID EntryPoint;
    } MODULEINFO, *LPMODULEINFO;
    extern "C" BOOL K32EnumProcesses(DWORD *lpidProcess, DWORD cb, LPDWORD lpcbNeeded);
    extern "C" BOOL K32EnumProcessModules(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded);
    extern "C" DWORD K32GetProcessImageFileNameA(HANDLE hProcess, LPSTR lpImageFileName, DWORD nSize);
    extern "C" DWORD K32GetProcessImageFileNameW(HANDLE hProcess, LPWSTR lpImageFileName, DWORD nSize);
    extern "C" BOOL K32GetModuleInformation(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb);
    #define EnumProcessModules K32EnumProcessModules
    #define EnumProcesses K32EnumProcesses
    // #define GetProcessImageFileNameA K32GetProcessImageFileNameA
    // #define GetProcessImageFileNameW K32GetProcessImageFileNameW
    #ifdef UNICODE
        #define GetProcessImageFileName K32GetProcessImageFileNameW
    #else
        #define GetProcessImageFileName K32GetProcessImageFileNameA
    #endif
    #define GetModuleInformation K32GetModuleInformation
#endif
#endif //PMO_WINDOWS_DEFINES_HPP
