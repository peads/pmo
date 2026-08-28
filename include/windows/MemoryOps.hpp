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

#include "../MemoryOps.hpp"
#include "types/ImportInfo.hpp"
#include "windows/ImageDirectoryEntryToData.hpp"

namespace PMO
{
    /**
     * @brief Searches for and returns first instance with given name if found in memory.
     * @details For each given name searches memory for first instance to return as a handle to the
     *          base address of same. Otherwise, returns NULL. names are assumed to be mutually
     *          exclusive.
     * @param names STL (Pseudo-)Container type holding names to search.
     * @param errBuf Buffer for returning human-readable error message, if not NULL. Defaults cerr.
     * @return Pointer to the starting address where the pattern was found, or NULL if not found.
     */
    template <PseudoContainer T, typename U>
    static HMODULE findModule(
        const T &names,
        std::basic_ostream<U> &errBuf = std::cerr
    )
    {
        for (auto name : names)
        {
            if (const HMODULE outModule = GetModuleHandle(name); outModule)
            {
                return outModule;
            }
        }

        errBuf << "Failed to find module in memory.\n";
        return nullptr;
    }

    inline MODULEINFO getImportInfo(const HMODULE module)
    {
        if (!module)
            return {};
        MODULEINFO info{};
        GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(MODULEINFO));
        return info;
    }

    // ReSharper disable once CppParameterMayBeConst
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
     * @param errBuf Buffer for returning human-readable error message. Defaults cerr.
     */
    template <typename T>
    static bool replaceCode(
        LPVOID addr,
        const char *code,
        const HMODULE *module,
        const size_t size,
        std::basic_ostream<T> &errBuf = std::cerr
    )
    {
        if (!addr || !code || !size)
        {
            errBuf << "Failed to flush cache.\n";
            return false;
        }

        DWORD flOldProtect;
        if (!VirtualProtect(addr, size,PAGE_EXECUTE_READWRITE, &flOldProtect))
        {
            errBuf << "Failed to set memory attributes.\n";
            return false;
        }
        memcpy(addr, code, size);
        if (!VirtualProtect(addr, size, flOldProtect, &flOldProtect))
        {
            errBuf << "Failed to reset memory attributes.\n";
            return false;
        }
        if (module)
            if (!FlushInstructionCache(*module, addr, size))
            {
                errBuf << "Failed to flush cache.\n";
                return false;
            }
        return true;
    }

    static void findThunks(const HMODULE module, const IMAGE_IMPORT_DESCRIPTOR &desc, ImportInfo &out)
    {
        auto thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(
            reinterpret_cast<PBYTE>(module) + desc.FirstThunk);
        for (; thunk->u1.AddressOfData; ++thunk)
        {
            out.funcs.push_back({.proc = reinterpret_cast<FARPROC>(thunk->u1.Function)});
        }
    }

    template <PseudoContainer T>
    static void findImportNames(const HMODULE module, T &out)
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
            ImportInfo bar{};
            bar.name = reinterpret_cast<PSTR>(
                reinterpret_cast<PBYTE>(module) + importDescriptor->Name);
            GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                              bar.name,
                              &bar.handle.module);
            findThunks(module, *importDescriptor, bar);
            out.push_back(bar);
        }
    }

     namespace Debug
     {
         inline void parseType(const MEMORY_BASIC_INFORMATION &mbi, std::iostream &out)
         {
             constexpr uint64_t masks[] = {MEM_IMAGE, MEM_MAPPED, MEM_PRIVATE};
             auto type = mbi.Type;
             if (!type)
             {
                 out << "MEM_FREE";
             }
             auto mPtr = masks;
             for (; type; type ^= *mPtr++)
             {
                 switch (type & *mPtr)
                 {
                     case MEM_IMAGE:
                         out << "MEM_IMAGE ";
                         break;
                     case MEM_MAPPED:
                         out << "MEM_MAPPED ";
                         break;
                     case MEM_PRIVATE:
                         out << "MEM_PRIVATE ";
                         break;
                     default:
                         out << "invalid value ";
                         break;
                 }
             }
         }
     }
        template <PseudoContainer T>
    static void findPatterns(
        MODULEINFO info,
        const char *pattern,
        const char *mask,
        const size_t patternLength,
        T &out
    )
    {
        return findPatterns(info.lpBaseOfDll, info.SizeOfImage, pattern, mask, patternLength, out);
    }

    template <PseudoContainer T>
    static void findPatterns(
        const HMODULE &module,
        const char *pattern,        // needn't be null-terminated (in fact, shouldn't be)
        const char *mask,           // must be null-terminated (i.e. it's a normal c-string)
        const size_t patternLength,
        T &out
    )
    {
        return findPatterns(getImportInfo(module), pattern, mask, patternLength, out);
    }
}
#endif //WMEMORYOPS_HPP
