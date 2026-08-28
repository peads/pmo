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

#include <cassert>
#include <vector>
#include <format>
#include <iostream>

#include "windows/MemoryOps.hpp"
#include "bmi2.hpp"

// IsDebuggerPresent use the pattern below
// 65 48 8B 04 25 60 00 00 00 0F B6 40 02 C3
#define IDP_PATTERN "\x65\x48\x8B\x04\x25\x60\x00\x00\x00\x0F\xB6\x40\x02\xC3"
#define IDP_MASK    "xxxxxxxxxxxxxx"
#define IDP_CODE    "\x31\xC0\xC3\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90"
// #define IDP_PLEN    14
// #define IDP_MLEN    14

// mov rax, qword ptr gs:[0x60] ...
// We'll use this bit and find all storage of value @ gs:[60h]
// into a register using a wildcard
#define GS60_PATTERN "\x65\x48\x8B\x04\x25\x60\x00\x00\x00"
#define GS60_MASK    "xxxxxx?xxxxxxxxxxx"
#define GS60_CODE    ""
// #define GS60_CODE    "\x31\xC0\xC3\x90\x90\x90\x90\x90\x90"
// #define GS60_PLEN    9
// #define GS60_MLEN    18

// #ifdef _MSC_VER
// #define bswap64(x) _byteswap_uint64(x)
// #else
// #define bswap64(x) __builtin_bswap64(x)
// #endif

typedef BOOL (*idpFn)();

template <PseudoContainer T>
void assertPatternsEq(const PMO::Pattern &pattern, const T &foundPatterns)
{
    for (const auto &e : foundPatterns)
    {
        std::cout << std::hex << std::uppercase << e << std::endl;
        char *ptr = reinterpret_cast<char *>(e);
        for (size_t i = 0; i < pattern.patternLen; ++i, ++ptr)
        {
            assert(*ptr == pattern.pattern.cptr[i]);
        }
    }
}

// template <PseudoContainer T>
// void printPatterns(const T &patterns, size_t patternLen)
// {
//     for (const auto e : patterns)
//         PMO::Debug::debug(e, patternLen);
// }

template <typename T, size_t N>
void generateBMI2Mask(const PMO::Pattern &pattern, T (&out)[N])
{
    size_t shift = pattern.maskLen > N;
    uint8_t F = shift ? 0xF : 0xFF;
    for (size_t i = 0; i < pattern.maskLen; ++i)
    {
        const auto n = shift ? (i & 1) << 2 : 0;
        if (!('?' ^ pattern.mask.cptr[i]))
        {
            out[i >> shift] |= F << n;
        }
    }
}

template <PseudoContainer T>
int testPextMask(const T &foundPatterns, const PMO::Pattern &pattern, const uint8_t *mask)
{
    for (const auto &e : foundPatterns)
    {
        const size_t len = pattern.patternLen;
        for (size_t i = 0; i < len; i += 16)
        {
            const auto n = *(uint64_t *)(e + i);
            const auto m = *(uint64_t *) (mask + i);
            assert(pext(n & m, m) == pext(n, m));
        }
    }
    return 0;
}

void testFindKnownPatternExistsInImports(const PMO::Pattern &pattern,
    const std::vector<PMO::ImportInfo> &imports, PMO::PointerUnion &knownPtr)
{
    std::vector<uintptr_t> foundPatterns{};

    for (const auto &[handle, name, funcs] : imports)
    {
        PMO::findPatterns(GetModuleHandle(name),
                          pattern.pattern.cptr,
                          pattern.mask.cptr,
                          pattern.patternLen,
                          foundPatterns);
    }

    // bool b, outB;
    // std::cout << "Is Debugger Present? " << std::boolalpha << IsDebuggerPresent() << std::endl;
    for (const auto &[handle, name, funcs] : imports)
    {
        for (const auto &f : funcs)
            if (f.address == reinterpret_cast<uintptr_t>(&IsDebuggerPresent))
            {
                // decode rva from jmp instruction
                const auto rva = *(uint64_t*)(f.cptr + 3) & 0xFFFFFFFF;
                const uintptr_t addr = f.address + rva + 7;

                std::cout << name << ": " << std::format("{:p} -> ", f.ptr);
                std::cout <<  std::format("{:016X} + {:016X} + 7 == {:016X}\n",
                    f.address, rva, addr);

                auto *fn = reinterpret_cast<idpFn*>(knownPtr.proc);
                char *p = *(char**)addr;    // it's a nested redirect. so, pointer->pointer
                char *q = pattern.pattern.cptr;
                char *r = (char*)fn;

                for (; *p != '\xC3'; ++q, ++p, ++r)
                {
                    const uint8_t a = *q & 0xFF;
                    const uint8_t b = *p & 0xFF;
                    const uint8_t c = *r & 0xFF;
                    assert(a == b && b == c);
                    // std::cout << std::format("{:02X} {:02X} {:02X}\n", a, b, c);
                }
            }
        // PMO::replaceCode<char>((LPVOID)e, pattern.code, nullptr, pattern.patternLen);
    }
    // std::cout << "Is Debugger Present? " << std::boolalpha << IsDebuggerPresent() << std::endl;
}

int main()
{
    const PMO::Pattern pattern{
        IDP_PATTERN,
        IDP_MASK,
        IDP_CODE
    };

    std::vector<uintptr_t> foundPatterns{};
    std::vector<PMO::ImportInfo> imports{};
    findImportNames(GetModuleHandle(nullptr), imports);

    // somewhere we know this pattern to be
    PMO::findPatterns(GetModuleHandle("KERNELBASE.dll"),
                          pattern.pattern.cptr,
                          pattern.mask.cptr,
                          pattern.patternLen,
                          foundPatterns);

    PMO::PointerUnion knownPtr{.address = foundPatterns.back()};
    assert(GetProcAddress(GetModuleHandle("KERNELBASE.dll"), "IsDebuggerPresent")
        == knownPtr.proc);
    std::cout << std::format("KERNELBASE.dll: {:p}\n", knownPtr.ptr);
    testFindKnownPatternExistsInImports(pattern, imports, knownPtr);

    const PMO::Pattern pattern1{
        GS60_PATTERN,
        GS60_MASK,
        GS60_CODE
    };

    uint8_t bmi2Mask[14] = {};
    generateBMI2Mask(pattern1, bmi2Mask);
    testPextMask(foundPatterns, pattern1, bmi2Mask);
    return 0;
}
