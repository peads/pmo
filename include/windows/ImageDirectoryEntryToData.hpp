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
#ifndef IMAGEDIRECTORYENTRYTODATA_HPP
#define IMAGEDIRECTORYENTRYTODATA_HPP

#include <windows.h>
#include <psapi.h>

inline PIMAGE_SECTION_HEADER GetSectionHeaderByRva(PIMAGE_NT_HEADERS headers, const ULONG addr) noexcept
{
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(headers);
    for (size_t i = 0; i < headers->FileHeader.NumberOfSections; ++i, ++section)
    {
        if (addr >= section->VirtualAddress &&
            addr < (section->VirtualAddress + section->Misc.VirtualSize))
        {
            return section;
        }
    }
    return nullptr;
}

// ReSharper disable CppParameterMayBeConst
// Reason: Windows types are stupid
inline PVOID ImageDirectoryEntryToDataEx(
    PVOID addr,
    const BOOLEAN isMappedAsImage,
    const USHORT entry,
    PULONG size,
    PIMAGE_SECTION_HEADER *header = nullptr
) noexcept
// ReSharper restore CppParameterMayBeConst
{
    if (addr && size)
    {
        if (const auto dosHeader = static_cast<PIMAGE_DOS_HEADER>(addr); IMAGE_DOS_SIGNATURE ==
            dosHeader->e_magic)
        {
            const auto ntHeaders =
                reinterpret_cast<PIMAGE_NT_HEADERS>(static_cast<PBYTE>(addr) + dosHeader->e_lfanew);
            if (IMAGE_NT_SIGNATURE == ntHeaders->Signature)
            {
                if (entry < ntHeaders->OptionalHeader.NumberOfRvaAndSizes)
                {
                    const ULONG rva = ntHeaders->OptionalHeader.DataDirectory[entry].VirtualAddress;

                    if (const ULONG dataSize = ntHeaders->OptionalHeader.DataDirectory[entry].Size;
                        rva && dataSize)
                    {
                        *size = dataSize;

                        // is image ==> rva is already valid virtual mem ptr
                        if (isMappedAsImage)
                        {
                            if (header)
                            {
                                *header = GetSectionHeaderByRva(ntHeaders, rva);
                            }
                            return static_cast<PBYTE>(addr) + rva;
                        }

                        // is flat file ==> convert rva to raw file offset using section headers
                        // ReSharper disable once CppLocalVariableMayBeConst
                        // Reason: Windows types are stupid
                        PIMAGE_SECTION_HEADER section = GetSectionHeaderByRva(ntHeaders, rva);
                        if (header)
                            *header = section;

                        if (!section)
                            return nullptr;

                        return static_cast<PBYTE>(addr) + rva - section->VirtualAddress +
                            section->PointerToRawData;
                    }
                }
            }
        }
    }
    return nullptr;
}
#endif // IMAGEDIRECTORYENTRYTODATA_HPP
