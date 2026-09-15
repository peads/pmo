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

#define PID_NAME_LEN 8192
#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define FORCE_INLINE_LAMBDA __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define FORCE_INLINE_LAMBDA [[msvc::forceinline]]
#else
    #define FORCE_INLINE_LAMBDA
#endif

namespace PMO
{
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

    inline bool findExports(
        const HMODULE &module,
        std::map<uintptr_t, std::tuple<WORD, std::string>> &out,
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

        size_t i = 0;
        for (; i < exportDirectory->NumberOfNames; ++i)
        {
            const auto ordinal = ordinals[i];
            if (const auto fn = baseAddress + addresses[ordinal]; !searchKey || *searchKey == fn)
            {
                out.emplace(fn, std::make_tuple(ordinal, std::string(reinterpret_cast<char*>(baseAddress + names[i]))));
                if (searchKey)
                    break;
            }
        }
        return i;
    }

    inline void findThunks(
        const HMODULE &module,
        const IMAGE_IMPORT_DESCRIPTOR &desc,
        ImportInfo &out
    ) noexcept
    {
        auto thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(
            reinterpret_cast<PBYTE>(module) + desc.FirstThunk);
        for (; thunk->u1.AddressOfData; ++thunk)
        {
            uintptr_t fn = thunk->u1.Function;
            uintptr_t gn = 0;

            if (const auto thisModule = GetModuleHandle(out.name); thisModule)
            {
                findExports(thisModule, out.exports, &fn);
                findNamedFunction(fn, &gn);
                out.thunks.emplace(fn, gn);
            }
        }
    }

    template <PseudoContainer T>
    inline void findImports(const HMODULE &module, T &out) noexcept
    {
        ULONG size;
        auto importDescriptor = static_cast<PIMAGE_IMPORT_DESCRIPTOR>(
            ImageDirectoryEntryToDataEx(module,
                                        TRUE,
                                        IMAGE_DIRECTORY_ENTRY_IMPORT,
                                        &size,
                                        nullptr));

        for (; importDescriptor && importDescriptor->Characteristics && importDescriptor->Name;
               ++importDescriptor)
        {
            ImportInfo imports{};
            imports.name = reinterpret_cast<PSTR>(
                reinterpret_cast<PBYTE>(module) + importDescriptor->Name);
            GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                              imports.name,
                              &imports.mod);
            findThunks(imports.mod, *importDescriptor, imports);
            out.push_back(imports);
        }
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

    static inline bool replaceCodeExternal(
        HANDLE proc,
        void *address,
        const Pattern &pattern
    ) noexcept
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
