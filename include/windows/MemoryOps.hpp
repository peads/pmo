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
#ifndef WMEMORYOPS_HPP
#define WMEMORYOPS_HPP

#include <filesystem>

#include "../MemoryOps.hpp"
#include "types/ImportInfo.hpp"
#include "windows/ImageDirectoryEntryToData.hpp"
// #if __has_include("syscalls.h")
// #include "syscalls.h"
// #endif

#define PID_NAME_LEN 8192
#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define FORCE_INLINE_LAMBDA __attribute__((always_inline))
#elif defined(_MSC_VER)
#define FORCE_INLINE_LAMBDA [[msvc::forceinline]]
#else
#define FORCE_INLINE_LAMBDA
#endif
#define NTDLL L"\x6E\x74\x64\x6C\x6C\x2E\x64\x6C\x6C"
#ifdef _WIN64
#define GET_PEB __readgsqword(0x60)
#else
#define GET_PEB __readfsdword(0x30)
#endif
#define PEB_THIS_MODULE_OFFSET 0x10
#define PEB_LDR_OFFSET 0x18
#define GET_FROM_OFFSET_PEB(off) *reinterpret_cast<void**>(GET_PEB + off)
#define getCurrentModule() static_cast<HMODULE>(GET_FROM_OFFSET_PEB(PEB_THIS_MODULE_OFFSET))
#define LDR_LIST_OFFSET 0x2
#define LDR_DLL_BASE_OFFSET 0x6
#define LDR_DLL_FULL_PATH_OFFSET 0x9
#define LDR_DLL_BASE_NAME_OFFSET 0xB

namespace PMO
{
    template <typename ModuleFunction>
    inline void getModule(const ModuleFunction &fun) noexcept requires std::is_invocable_v<ModuleFunction, void**>
    {
        void *ldr = GET_FROM_OFFSET_PEB(PEB_LDR_OFFSET);

        for (void   **cend = static_cast<void**>(ldr) + LDR_LIST_OFFSET,
                    **curr = static_cast<void**>(*cend);
             curr != cend;
             curr = static_cast<void**>(*curr))
        {
            if (fun(curr))
                break;
        }
    }

    template <typename T>
    static inline void toLower(std::basic_string<T> &text) noexcept
    {
        std::ranges::transform(text, text.begin(),
        [](const unsigned char c)FORCE_INLINE_LAMBDA
        {
            return std::tolower(c);
        });
    }

    inline HMODULE getModule(const wchar_t *wname) noexcept
    {
        HMODULE result = nullptr;
        std::wstring key(wname);
        toLower(key);
        getModule([&result, &key](void **curr)FORCE_INLINE_LAMBDA
        {
            // const UNICODE_STRING name = *reinterpret_cast<UNICODE_STRING*>(reinterpret_cast<
                // uintptr_t>(curr) + 88);
            std::wstring buf(*reinterpret_cast<wchar_t**>(curr + LDR_DLL_BASE_NAME_OFFSET + 1));
            toLower(buf);
            if (buf == key) // curr->BaseDllName == wname
            {
                result = *reinterpret_cast<HMODULE*>(curr + LDR_DLL_BASE_OFFSET);
                // result = *reinterpret_cast<HMODULE*>(reinterpret_cast<uintptr_t>(curr) + 48); // curr->DllBase
                return true;
            }
            return false;
        });
        return result;
    }

    inline HMODULE getModule(const char *name) noexcept
    {
        size_t len = 0;
        wchar_t buffer[MAX_PATH];
        mbstowcs_s(&len, buffer, MAX_PATH, name, MAX_PATH);
        buffer[len] = L'\0';
        return getModule(buffer);
    }

    template <PseudoContainer T = std::vector<HMODULE>>
    inline auto getModules() noexcept requires std::is_same_v<typename T::value_type, HMODULE>
    {
        T result{};
        getModule([&result](void **curr)FORCE_INLINE_LAMBDA
        {
            result.push_back(*reinterpret_cast<HMODULE*>(reinterpret_cast<uintptr_t>(curr) + 48));
            return false;
        });
        return std::move(result);
    }

    inline auto getModuleFileName(HMODULE module) noexcept
    {
        wchar_t *result = nullptr;
        getModule([&result, &module](void **curr)
        {
            if (*reinterpret_cast<HMODULE*>(curr + LDR_DLL_BASE_OFFSET) == module)
            {
                result = *reinterpret_cast<wchar_t**>(curr + LDR_DLL_BASE_NAME_OFFSET + 1);
                return true;
            }
            return false;
        });
        return std::move(std::wstring(result));
    }
    // TODO change these to wrappers passing the offset required
    inline auto getModuleFullPath(HMODULE module) noexcept
    {
        wchar_t *result = nullptr;
        getModule([&result, &module](void **curr)
        {
            if (*reinterpret_cast<HMODULE*>(curr + LDR_DLL_BASE_OFFSET) == module)
            {
                result = *reinterpret_cast<wchar_t**>(curr + LDR_DLL_FULL_PATH_OFFSET + 1);
                return true;
            }
            return false;
        });
        return std::move(std::wstring(result));
    }

    /**
     * @brief Searches for and returns first instance with given name if found in memory.
     * @details For each given name searches memory for first instance to return as a handle to the
     *          base address of same. Otherwise, returns NULL. names are assumed to be mutually
     *          exclusive.
     * @param names STL (Pseudo-)Container type holding names to search.
     * @return Pointer to the starting address where the pattern was found, or NULL if not found.
     */
    template <size_t N>
    inline HMODULE findModule(const char *(&names)[N]) noexcept
    {
        for (auto name : names)
            if (const HMODULE outModule = GetModuleHandle(name); outModule)
                return outModule;
        return nullptr;
    }

    inline bool getImportInfo(const HMODULE &module, MODULEINFO &info) noexcept
    {
        if (!module)
            return false;
        return GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(MODULEINFO));
    }

    inline MODULEINFO getImportInfo(const HMODULE &module) noexcept
    {
        MODULEINFO info{};
        getImportInfo(module, info);
        return info;
    }

    /**
     * @brief Replaces the extant code at given memory address with code bytes at the given pointer,
     *          and returns success as boolean.
     * @details Guards against null pointers for addr, code, module, and zero-length. memcpy is used
     *          for speed. Thus, addr and code pointers must not contain overlapping memory regions,
     *          or the behavior is undefined. Clears error buffer if successful. A stream can be
     *          provided for more verbose error detail.
     * @param addr Memory address where the code to-be-replaced starts. May not be NULL.
     * @param code Pointer to the contiguous bytes (e.g. an array) to be written to
     *             addr. May not be NULL.
     * @param module Module containing addr. May not be NULL;
     * @param size Number of bytes to be replaced.
     */
    inline bool replaceCode(
        const PointerUnion &addr,
        const char *const code,
        const size_t size,
        const HMODULE *module = nullptr
    ) noexcept
    {
        if (!addr.address || !code || !size)
            return false;

        DWORD flOldProtect;

        if (!VirtualProtect(addr.ptr, size, PAGE_EXECUTE_READWRITE, &flOldProtect))
            return false;

        memcpy(addr.ptr, code, size);

        if (!VirtualProtect(addr.ptr, size, flOldProtect, &flOldProtect))
            return false;

        if (module && !FlushInstructionCache(*module, addr.ptr, size))
            return false;

        return true;
    }

    // TODO consider changing search key to a boolean function lambda
    inline DWORD findExports(
        const HMODULE &module,
        std::map<WORD, std::tuple<uintptr_t, char*, bool>> &out,
        const uintptr_t *searchKey = nullptr
    ) noexcept
    {
        if (!module)
            return false;

        const auto baseAddress = reinterpret_cast<uintptr_t>(module);
        const auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(baseAddress);

        if (dosHeader->e_magic ^ IMAGE_DOS_SIGNATURE)
            return false;

        const auto [virtualAddress, size]
            = reinterpret_cast<PIMAGE_NT_HEADERS>(baseAddress + dosHeader->e_lfanew)
                ->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

        if (!virtualAddress)
            return false;

        const auto exportDirectory
            = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(baseAddress + virtualAddress);
        const auto addresses
            = reinterpret_cast<PDWORD>(baseAddress + exportDirectory->AddressOfFunctions);
        const auto names = reinterpret_cast<PDWORD>(baseAddress + exportDirectory->AddressOfNames);
        const auto ordinals
            = reinterpret_cast<PWORD>(baseAddress + exportDirectory->AddressOfNameOrdinals);

        for (size_t i = 0; i < exportDirectory->NumberOfNames; ++i)
        {
            const auto ordinal = ordinals[i];
            if (const auto fn = baseAddress + addresses[ordinal]; !searchKey || *searchKey == fn)
            {
                out.emplace(ordinal,
                    std::make_tuple(fn, reinterpret_cast<char*>(baseAddress + names[i]), true));
                if (searchKey)
                    break;
            }
        }
        if (searchKey)
            return exportDirectory->Base;

        for (size_t i = 0; i < exportDirectory->NumberOfFunctions; ++i)
        {
            auto ord = static_cast<WORD>(i);
            if (out.contains(ord)) continue;
            out.insert({ord, std::make_tuple(baseAddress + addresses[i],
                MAKEINTRESOURCE(exportDirectory->Base + ord), false)});
        }
        return exportDirectory->Base;
    }

    template <size_t N>
    inline auto findProcByName(const char (&inName)[N], const std::map<WORD, std::tuple<uintptr_t, char*, bool>> &exports) noexcept
    {
        return std::move(exports | std::views::values | std::views::filter([&inName](auto &e)
        {
            auto &[fn, name, isNamed] = e;
            return isNamed ? std::string(name) == inName : false;
        }) | std::views::keys);
    }

    inline DWORD findThunks(
        const HMODULE &module,
        // const IMAGE_IMPORT_DESCRIPTOR &desc,
        const IMAGE_THUNK_DATA *thunk,
        ImportInfo &out
    ) noexcept
    {
        DWORD result = 0;
        for (; thunk->u1.AddressOfData; ++thunk)
        {
            // if (const auto thisModule = GetModuleHandle(out.name.c_str()); thisModule)
            // {
                uintptr_t fn = thunk->u1.Function;
                uintptr_t gn = 0;
                result = findExports(module, out.exports, &fn);
                findNamedFunction(fn, &gn);
                out.thunks.emplace(fn, gn);
            // }
        }
        return result;
    }

    template <PseudoContainer T>
    inline void findImports(const HMODULE &module, T &out) noexcept requires std::is_same_v<typename T::value_type, ImportInfo>
    {
        ULONG size;
        auto desc = static_cast<PIMAGE_IMPORT_DESCRIPTOR>(
            ImageDirectoryEntryToDataEx(module,
                                        TRUE,
                                        IMAGE_DIRECTORY_ENTRY_IMPORT,
                                        &size,
                                        nullptr));

        ImportInfo info{};
        for (; desc && desc->Characteristics && desc->Name; ++desc)
        {
            const auto name = reinterpret_cast<PSTR>(reinterpret_cast<PBYTE>(module) + desc->Name);
            GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                              name,
                              &info.mod);
            info.name = std::string(name);
            const auto thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(reinterpret_cast<PBYTE>(module) + desc->FirstThunk);
            info.ordinalBase = findThunks(GetModuleHandle(name), thunk, info);
            out.push_back(std::move(info));
        }
    }

    template <SetConcept T = SetWrapper<ImportInfo>>
    inline T getImports() noexcept requires std::is_same_v<typename T::value_type, ImportInfo>
    {
        T result{};
        char buffer[MAX_PATH];

        getModule([&result, &buffer](void **curr)FORCE_INLINE_LAMBDA
        {
            // const auto aCurr = reinterpret_cast<uintptr_t>(curr);
            const auto module = *reinterpret_cast<HMODULE*>(curr + LDR_DLL_BASE_OFFSET); // curr->DllBase
            // const UNICODE_STRING wname = *reinterpret_cast<UNICODE_STRING*>(aCurr + 88);

            size_t len = 0;
            wcstombs_s(&len, buffer, *reinterpret_cast<wchar_t**>(curr + LDR_DLL_BASE_NAME_OFFSET + 1), MAX_PATH);
            buffer[len] = '\0';

            ULONG size;
            auto desc = static_cast<PIMAGE_IMPORT_DESCRIPTOR>(
                ImageDirectoryEntryToDataEx(module,
                                            TRUE,
                                            IMAGE_DIRECTORY_ENTRY_IMPORT,
                                            &size,
                                            nullptr));

            for (; desc && desc->Characteristics && desc->Name; ++desc)
            {
                const auto name = reinterpret_cast<PSTR>(reinterpret_cast<PBYTE>(module) + desc->Name);
                auto mod = getModule(name);
                // ImportInfo info{.mod = mod ? mod : module, .name = std::string(buffer)};
                ImportInfo info{.mod = mod ? mod : module, .name = name};
                const auto thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(reinterpret_cast<PBYTE>(module) + desc->FirstThunk);
                info.ordinalBase = findThunks(mod, thunk, info);
                if (!result.contains(info))
                    result.push_back(std::move(info));
                else
                {
                    ImportInfo val = *result.find(info);
                    val.exports.merge(info.exports);
                    val.thunks.merge(info.thunks);
                    // if (!val.mod)
                        // val.mod = mod ? mod : module;
                }
            }

            return false;
        });

        return std::move(result);
    }

    // no it doesn't. ReSharper can't even reference types in structs
    // ReSharper disable once CppDFAConstantFunctionResult
    inline bool findPatternsExternal(
        const DWORD &pid,
        Pattern &searchStruct,
        const bool stopOne = false
    ) noexcept
    {
        const auto proc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (!proc)
        {
            return false;
        }
        bool result = false;
        MEMORY_BASIC_INFORMATION mbi;
        std::vector<char> buffer(8192);

        const auto end = searchStruct.pattern.cptr + searchStruct.patternLen;
        const PointerUnion theEnd = {.cptr = end};

        for (uintptr_t address = 0;
             VirtualQueryEx(proc, reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi)) == sizeof(
                 mbi); address = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize)
        {
            if (mbi.State == MEM_COMMIT && (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ |
                PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)))
            {
                size_t bytesRead = 0;
                if (buffer.size() < mbi.RegionSize)
                    buffer.resize(mbi.RegionSize);
                if (ReadProcessMemory(proc,
                                      mbi.BaseAddress,
                                      buffer.data(),
                                      mbi.RegionSize,
                                      &bytesRead))
                {
                    size_t rva = 0;
                    PointerUnion pointer = {.cptr = buffer.data()};
                    SearchContext ctx{
                        .theEnd = theEnd,
                        .offset = reinterpret_cast<uintptr_t>(mbi.BaseAddress) - pointer.address,
                        .pointer = pointer,
                        .len = rva,
                        .searchStruct = searchStruct,
                        .result = result,
                    };
                    // TODO figure out if shift is applied during this traversal
                    while (pointer.cptr < bytesRead + buffer.data())
                        if (searchChunked(ctx) && stopOne)
                            goto finished;
                }
            }
        }
finished:
        CloseHandle(proc);
        return result;
    }

    inline uintptr_t getProcessesByName(const std::string &key, std::vector<DWORD> &out) noexcept
    {
        uintptr_t result = 0;
        DWORD cbNeeded;
        size_t size = PID_NAME_LEN * sizeof(DWORD);
        // TODO consider reimplementing EnumProcesses with calls to NtQuerySystemInformation
        if (DWORD pids[PID_NAME_LEN]; EnumProcesses(pids, size, &cbNeeded) && cbNeeded <= size)
        {
            // TODO consider re-processing with larger storage e.g.,
            //    std::vector<DWORD, cbNeeded / sizeof(DWORD)>{};
            const size_t len = cbNeeded / sizeof(DWORD);
            if (len > PID_NAME_LEN)
                return -1;
            // HMODULE modules[LEN];
            size = PID_NAME_LEN * sizeof(HMODULE);

            for (size_t i = 0; i < len; ++i)
            {
                if (0xCCCC'CCCC == pids[i])
                    continue;
                // ReSharper disable once CppLocalVariableMayBeConst
                HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                          FALSE,
                                          pids[i]);
                if (char name[MAX_PATH]; proc && GetProcessImageFileName(proc, name, MAX_PATH))
                {
                    if (std::filesystem::path path(static_cast<const char*>(name)); path.
                        has_filename() && path.filename().string() == key)
                    {
                        out.push_back(pids[i]);
                        MODULEINFO modInfo;
                        // Enumerate process modules
                        if (HMODULE mods[1024]; EnumProcessModules(proc,
                                mods,
                                sizeof(mods),
                                &cbNeeded)
                            && GetModuleInformation(proc, mods[0], &modInfo, sizeof(modInfo)))
                        {
                            result = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
                        }
                    }
                }
                CloseHandle(proc);
            }
        }
        return result;
    }

    // ReSharper disable once CppParameterMayBeConst
    static inline bool replaceCodeExternal(HANDLE proc, void *address, const Pattern &pattern) noexcept
    {
        return WriteProcessMemory(proc, address, pattern.code.ptr, pattern.codeLen, nullptr);
    }

    // ReSharper disable once CppParameterMayBeConst
    inline bool replaceCodeExternal(HANDLE proc, Pattern &pattern) noexcept
    {
        DWORD exitCode = 0;
        if (!proc || pattern.empty() || GetExitCodeProcess(proc, &exitCode) && STILL_ACTIVE !=
            exitCode)
        {
            return false;
        }

        return replaceCodeExternal(proc, pattern.back().ptr, pattern);
    }

    inline bool replaceAllCodeExternal(HANDLE proc, Pattern &pattern) noexcept
    {
        return std::ranges::all_of(pattern,
       [&proc, &pattern](const uintptr_t addr)
       {
           return replaceCodeExternal(proc, pattern);
       });
    }
}
#endif //WMEMORYOPS_HPP
