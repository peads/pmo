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

#include <deque>
#include <format>
#include <ranges>

#include "windows/MemoryOps.hpp"
#include <catch2/catch_test_macros.hpp>

#define CATCH_CONFIG_MAIN // provides main(); this line is required in only one .cpp file

// IsDebuggerPresent uses the pattern below
#define IDP_PATTERN "\x65\x48\x8B\x04\x25\x60\x00\x00\x00\x0F\xB6\x40\x02\xC3"
#define IDP_MASK    "x?x?xxxxxxxxxx"
// #define IDP_MASK    "xx?xxxx?xxxxxxxxxxxxxxxxxxxx"
#define IDP_CODE    "\x31\xC0\xC3\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90"
// CheckDebuggerPresent uses the pattern below
#define CRDP_PATTERN "\x48\x8B\xC4\x48\x89\x58\x10\x57\x48\x83\xEC\x30\x33\xDB\x48\x8B\xFA\x48\x89\x58\x08\x48\x85\xC9\x74\x3F\x48\x85\xD2\x74\x3A\x44\x8D\x4B\x08\x48\x89\x58\xE8\x4C\x8D\x40\x08\x8D\x53\x07\x48\xFF\x15\xA3\x99\x19\x00\x0F\x1F\x44\x00\x00\x85\xC0\x78\x30\x48\x39\x5C\x24\x40\xB8\x01\x00\x00\x00\x0F\x95\xC3\x89\x1F\x48\x8B\x5C\x24\x48\x48\x83\xC4\x30\x5F\xC3"
#define CRDP_MASK    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define CRDP_CODE    "\xB8\x01\x00\x00\x00\x31\xd2\xC3"
// mov rax, qword ptr gs:[0x60] ...
// We'll use this to find all storage of value @ gs:[60h]
// into a register using a wildcard
#define GS60_PATTERN "\x65\x48\x8B\x04\x25\x60\x00\x00\x00"
// #define GS60_MASK    "x?x?xxxxx"
#define GS60_MASK    "xx?xxxx?xxxxxxxxxx"

static const PMO::Pattern patterns[] = {
    PMO::Pattern{IDP_PATTERN,IDP_MASK,IDP_CODE},
    PMO::Pattern{CRDP_PATTERN,CRDP_MASK,CRDP_CODE},
    PMO::Pattern{GS60_PATTERN,GS60_MASK, ""}
};

template <size_t N>
static uintptr_t parseJmpFar(uint8_t (&code)[N], const uintptr_t rip)
{
    uint64_t result = 0;
    uint8_t *p = &reinterpret_cast<uint8_t*>(&code)[0];
    uint8_t *q = &reinterpret_cast<uint8_t*>(&result)[0];
    bool atRva = false;
    for (size_t i = 0; i < N; ++i)
    {
        switch (*p)
        {
            case 0x48:
            case 0xFF:
                ++p;
                break;
            default:
                if (atRva)
                {
                    // std::cout << std::format("{:02X}\n", *p);
                    *q++ = *p++;
                }
                else
                {
                    switch (*p & 0xF) // @ 48 FF xy now, don't care about x
                    {
                        case 0x5:   // 5 is jmp far absolute indirect
                            atRva = true;
                            ++p;
                            i = N - 5;
                            break;
                        default:
                            return false;
                    }
                }
        }
    }
    return result + rip + 7;
}

TEST_CASE("000a more pdep/pext search testing", "[PMO]")
{
    PMO::SetWrapper<PMO::ImportInfo> imports{};
    findImports(GetModuleHandle(nullptr), imports);

    for (auto &pattern : patterns)
    {
        bool foundAtLeastOne = false;
        for (auto it = imports.begin(); it != imports.end(); ++it)
        {
            auto module = GetModuleHandle(it->name);
            findImports(module, imports);
            MODULEINFO info{};
            PMO::getImportInfo(module, info);
            foundAtLeastOne |= findPatterns(reinterpret_cast<uintptr_t>(info.
                                                                 lpBaseOfDll),
                                                             info.SizeOfImage,
                                                             pattern,
                                                             *pattern.occurrences);
        }
        REQUIRE(foundAtLeastOne);
        std::cout << std::dec << "Found: " << pattern.occurrences->size() << std::endl;
    }
}

TEST_CASE("000 pdep/pext search testing", "[PMO]")
{
    uintptr_t idp;
    const auto addr = reinterpret_cast<uintptr_t>(&IsDebuggerPresent);
    PMO::findNamedFunction(addr, &idp);
    // const size_t len = pattern.patternLen;
    PMO::SetWrapper<uintptr_t> funcs{};
    REQUIRE(PMO::findPatterns(*reinterpret_cast<uintptr_t*>(idp),
                patterns[0].patternLen, patterns[0], funcs));

    auto module = GetModuleHandle("KERNELBASE.dll");
    MODULEINFO info{};
    PMO::getImportInfo(module, info);
    REQUIRE(PMO::findPatterns(reinterpret_cast<uintptr_t>(info.lpBaseOfDll), info.
                SizeOfImage, patterns[0], funcs));
    REQUIRE(reinterpret_cast<int(*)()>(funcs.back())() == IsDebuggerPresent());
    PMO::SetWrapper<PMO::ImportInfo> imports{};
    findImports(GetModuleHandle(nullptr), imports);
    bool foundAtLeastOne = false;
    for (auto it = imports.begin(); it != imports.end(); ++it)
    {
        module = GetModuleHandle(it->name);
        findImports(module, imports);
        PMO::getImportInfo(module, info);
        foundAtLeastOne |= findPatterns(reinterpret_cast<uintptr_t>(info.
                                                             lpBaseOfDll),
                                                         info.SizeOfImage,
                                                         patterns[0],
                                                         funcs);
    }
    REQUIRE(foundAtLeastOne);
    for (auto &f : funcs)
    {
        REQUIRE((reinterpret_cast<int(*)()>(f))() == IsDebuggerPresent());
    }
    REQUIRE((*reinterpret_cast<int(**)()>(idp))() == IsDebuggerPresent());
    REQUIRE(reinterpret_cast<int(*)()>(addr)() == IsDebuggerPresent());
    funcs.clear();
}

TEST_CASE("004 Test expected function name", "[PMO]")
{
    std::deque<PMO::ImportInfo> imports;
    findImports(GetModuleHandle(nullptr), imports);
    const auto idp = reinterpret_cast<uintptr_t>(&IsDebuggerPresent);
    const auto crdp = reinterpret_cast<uintptr_t>(&CheckRemoteDebuggerPresent);

    for (const auto &im : imports)
    {
        if (!strcmp("KERNEL32.dll", im.name))
        {
            REQUIRE(im.fnNames.at(idp)=="IsDebuggerPresent");
            REQUIRE(im.fnNames.at(crdp)=="CheckRemoteDebuggerPresent");
            break;
        }
    }
}

TEST_CASE("003 Test replace by function name", "[PMO]")
{
    int outB = 0;

    int (*idp)() = nullptr;
    auto addr = reinterpret_cast<uintptr_t>(&IsDebuggerPresent);
    PMO::findNamedFunction(addr, &idp);
    // ReSharper disable once CppCStyleCast
    patterns[0].occurrences->push_back((uintptr_t) idp);
    // int (*idp)() = *reinterpret_cast<int(**)()>(ptr);
    REQUIRE(idp() == IsDebuggerPresent());

    int (*crdp)(HANDLE, int *) = nullptr;
    addr = reinterpret_cast<uintptr_t>(&CheckRemoteDebuggerPresent);
    PMO::findNamedFunction(addr, &crdp);
    // ReSharper disable once CppCStyleCast
    patterns[1].occurrences->push_back((uintptr_t) crdp);
    // int (*crdp)(HANDLE, int *) = *reinterpret_cast<int(**)(HANDLE, int *)>(ptr);
    int outBB = 0;
    REQUIRE(1 == crdp(GetCurrentProcess(), &outBB));
    REQUIRE(1 == CheckRemoteDebuggerPresent(GetCurrentProcess(), &outB));
    REQUIRE(outBB == outB);

    for (const auto &e : {patterns[0], patterns[1]})
    {
        for (const auto &p : *e.occurrences)
        {
            PMO::replaceCode<char>(p, e);
            char *code = reinterpret_cast<char*>(p);
            for (size_t i = 0; i < e.codeLen; ++i, ++code)
            {
                REQUIRE(e.code.cptr[i] == *code);
            }
        }
    }

    outB = 0;
    REQUIRE(!IsDebuggerPresent());
    REQUIRE(1 == crdp(GetCurrentProcess(), &outBB));
    REQUIRE(1 == CheckRemoteDebuggerPresent(GetCurrentProcess(), &outB));
    REQUIRE(!outB);
}

TEST_CASE("002 Test find by traversing thunks", "[PMO]")
{
    // REQUIRE(FreeLibrary(GetModuleHandle("KERNEL32.dll")) != 0);
    // HMODULE hDll = LoadLibrary("KERNEL32.dll");
    // REQUIRE(hDll != nullptr);

    auto addr = reinterpret_cast<uintptr_t>(&IsDebuggerPresent);
    int (*fn)() = nullptr;
    PMO::findNamedFunction(addr, &fn);
    HMODULE module = GetModuleHandle(nullptr);
    REQUIRE(module);

    std::deque<PMO::ImportInfo> imports;
    findImports(module, imports);

    const auto pattern = PMO::Pattern{IDP_PATTERN,IDP_MASK,IDP_CODE};

    std::deque<uintptr_t> funcs;
    for (auto &im : imports)
    {
        for (auto &gn : im.thunks | std::views::values)
        {
            if (gn == addr)
            {
                auto ptr = reinterpret_cast<void**>(gn);
                findPatterns(reinterpret_cast<uintptr_t>(*ptr), pattern.patternLen, pattern, funcs);
                break;
            }
        }
        if (!funcs.empty())
            break;
    }
    int (*fn1)() = &IsDebuggerPresent;
    REQUIRE(fn() == fn1());
    int (*fn2)() = *reinterpret_cast<int(*)()>(funcs.back());
    REQUIRE(fn2() == fn1());
}

TEST_CASE("001 Test parse far jmp", "[PMO]")
{
    uint8_t code[8];
    char *ptr = reinterpret_cast<char*>(&IsDebuggerPresent);
    for (size_t i = 0; *ptr != '\xCC'; ++ptr, ++i)
    {
        code[i] = *ptr;
    }

    auto addr = reinterpret_cast<uintptr_t>(&IsDebuggerPresent);
    const auto out = parseJmpFar(code, addr);
    int (*fn)() = nullptr;
    const auto outTest = PMO::findNamedFunction(addr, &fn);

    REQUIRE((addr - reinterpret_cast<uintptr_t>(&IsDebuggerPresent) - 7) == outTest);
    REQUIRE(addr == out);
    REQUIRE(fn() == IsDebuggerPresent());
}
