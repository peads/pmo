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
#include <map>
#include <ranges>

#include "main.hpp"

#include <set>

#include "debug.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/internal/catch_stdstreams.hpp>

#define CATCH_CONFIG_MAIN // provides main(); this line is required in only one .cpp file

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

static inline void reset()
{
    for (auto &pattern : debuggerPatterns)
        pattern.reset();
}

TEST_CASE("05 line coverage++", "[PMO]")
{
    reset();
    std::set<PMO::PointerUnion> puSet{};
    std::unordered_map<PMO::PointerUnion, std::string> puMap{};
    std::map<PMO::PointerUnion, std::string> puTree{};
    PMO::SetWrapper<PMO::PointerUnion> puSwU{};

    std::unordered_set<PMO::ImportInfo> imSet{};
    std::unordered_map<PMO::ImportInfo, std::string> imMap{};
    std::map<PMO::ImportInfo, std::string> imTree{};
    PMO::SetWrapper<PMO::PointerUnion, std::set<PMO::ImportInfo>> imSwO{};

    for (size_t i = 0; i < 5; ++i)
    {
        auto val = 0x12345678 + i;
        auto pu = PMO::PointerUnion{.address = val};
        auto str = std::format("{:016X}", val);
        auto im = PMO::ImportInfo{.name = str.c_str()};
        puSet.insert(pu);
        puMap.insert_or_assign(pu, str);
        puTree.insert_or_assign(pu, str);
        puSwU.insert(pu);

        imSet.insert(im);
        imMap.insert_or_assign(im, str);
        imTree.insert_or_assign(im, str);
        imSwO.insert(im);
    }
    uintptr_t addr;
    REQUIRE((!PMO::findNamedFunction(0, &addr) && !addr));
}

TEST_CASE("00a more raw search testing", "[PMO]")
{
    reset();
    PMO::SetWrapper<PMO::ImportInfo> imports{};
    // findImports(GetModuleHandle(nullptr), imports);
    char buf[260];
    GetModuleFileName(nullptr, buf, MAX_PATH);
    std::filesystem::path path{buf};
    const std::string name = path.filename().string();
    imports.insert(PMO::ImportInfo{.name = name.data()});
    constexpr size_t expected[] = {1, 1};

    size_t cnt = 0;
    for (auto &pattern : debuggerPatterns)
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
                                            pattern);
        }
        REQUIRE(foundAtLeastOne);
        REQUIRE(pattern.size() >= expected[cnt++]);
    }
}

TEST_CASE("00 raw search testing", "[PMO]")
{
    reset();
    uintptr_t idp;
    const auto addr = idpAddr;
    PMO::findNamedFunction(addr, &idp);
    REQUIRE(PMO::findPatterns(*reinterpret_cast<uintptr_t*>(idp),
                debuggerPatterns[0].patternLen, debuggerPatterns[0]));

    auto module = GetModuleHandle("KERNELBASE.dll");
    MODULEINFO info{};
    PMO::getImportInfo(module, info);
    REQUIRE(PMO::findPatterns(reinterpret_cast<uintptr_t>(info.lpBaseOfDll), info.
                SizeOfImage, debuggerPatterns[0]));
    REQUIRE(reinterpret_cast<int(*)()>(debuggerPatterns[0].back().address)() == IsDebuggerPresent(
            ));
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
                                        debuggerPatterns[0]);
    }
    REQUIRE(foundAtLeastOne);
    for (unsigned long long f : debuggerPatterns[0])
    {
        REQUIRE((reinterpret_cast<int(*)()>(f))() == IsDebuggerPresent());
    }
    REQUIRE((*reinterpret_cast<int(**)()>(idp))() == IsDebuggerPresent());
    REQUIRE(reinterpret_cast<int(*)()>(addr)() == IsDebuggerPresent());
}

TEST_CASE("04 Test expected function name", "[PMO]")
{
    reset();
    std::deque<PMO::ImportInfo> imports;
    findImports(GetModuleHandle(nullptr), imports);
    const auto idp = idpAddr;
    const auto crdp = crdpAddr;

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

TEST_CASE("ZZ Test replace by function name", "[PMO]")
{
    reset();
    int outB = 0;

    int (*idp)() = nullptr;
    auto addr = idpAddr;
    PMO::findNamedFunction(addr, &idp);
    // ReSharper disable once CppCStyleCast
    debuggerPatterns[0].push_back((uintptr_t) idp);
    // int (*idp)() = *reinterpret_cast<int(**)()>(ptr);
    REQUIRE(idp() == IsDebuggerPresent());

    int (*crdp)(HANDLE, int *) = nullptr;
    addr = crdpAddr;
    PMO::findNamedFunction(addr, &crdp);
    // ReSharper disable once CppCStyleCast
    debuggerPatterns[1].push_back((uintptr_t) crdp);
    // int (*crdp)(HANDLE, int *) = *reinterpret_cast<int(**)(HANDLE, int *)>(ptr);
    int outBB = 0;
    REQUIRE(1 == crdp(GetCurrentProcess(), &outBB));
    REQUIRE(1 == CheckRemoteDebuggerPresent(GetCurrentProcess(), &outB));
    REQUIRE(outBB == outB);

    auto knrlBase = GetModuleHandle("KERNELBASE.dll");
    auto cnt = 0;
    for (const auto &e : debuggerPatterns)
    {
        for (const auto &p : e)
        {
            if (cnt++)
                PMO::replaceCode({.address = p}, e.code.str, e.codeLen);
            else
                PMO::replaceCode({.address = p}, e.code.str, e.codeLen, &knrlBase);
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

TEST_CASE("02 Test find by traversing thunks", "[PMO]")
{
    reset();
    auto addr = idpAddr;
    int (*fn)() = nullptr;
    PMO::findNamedFunction(addr, &fn);
    HMODULE module = GetModuleHandle(nullptr);
    REQUIRE(module);

    std::deque<PMO::ImportInfo> imports;
    findImports(module, imports);

    // const auto pattern = PMO::Pattern{IDP_PATTERN,IDP_MASK,IDP_CODE};

    for (auto &im : imports)
    {
        for (auto &gn : im.thunks | std::views::values)
        {
            if (gn == addr)
            {
                findPatterns(reinterpret_cast<uintptr_t>(*reinterpret_cast<void**>(gn)),
                             debuggerPatterns[0].patternLen,
                             debuggerPatterns[0]);
                break;
            }
        }
        if (!debuggerPatterns[0].empty())
            break;
    }
    int (*fn1)() = &IsDebuggerPresent;
    REQUIRE(fn() == fn1());
    int (*fn2)() = *reinterpret_cast<int(*)()>(debuggerPatterns[0].back().address);
    REQUIRE(fn2() == fn1());
}

TEST_CASE("01 Test parse far jmp", "[PMO]")
{
    reset();

    auto addr = idpAddr;
    auto out = parseJmpFar(*reinterpret_cast<uint8_t(*)[8]>(&IsDebuggerPresent), addr);
    int (*fn)() = nullptr;
    auto outTest = PMO::findNamedFunction(addr, &fn);

    REQUIRE((addr - idpAddr - 7) == outTest);
    REQUIRE(addr == out);
    REQUIRE(fn() == IsDebuggerPresent());

    addr = crdpAddr;
    out = parseJmpFar(*reinterpret_cast<uint8_t(*)[8]>(&CheckRemoteDebuggerPresent), addr);
    int (*fn1)(HANDLE, int *) = nullptr;
    outTest = PMO::findNamedFunction(addr, &fn1);

    REQUIRE((addr - crdpAddr - 7) == outTest);
    REQUIRE(addr == out);
    int b[2];
    REQUIRE(fn1(GetCurrentProcess(), b) == CheckRemoteDebuggerPresent(GetCurrentProcess(), b + 1));
    REQUIRE(b[0] == b[1]);
}

TEST_CASE("03 Test findProcessByName", "[PMO]")
{
    std::vector<DWORD> handles{};
    char buffer[MAX_PATH];
    GetModuleFileName(nullptr, buffer, MAX_PATH);
    const std::filesystem::path path(buffer);
    REQUIRE((path.has_filename() && "pmo.exe" == path.filename().string()));
    PMO::getProcessesByName(path.filename().string(), handles);
    REQUIRE((!handles.empty() && handles.back() == GetCurrentProcessId()));
}

TEST_CASE("06 Optional Test 6 test find code in memory of external process", "[PMO]")
{
    std::vector<DWORD> pids{};
    for (const auto &e : {OBR_WIN64, OBR_WINGDK})
    {
        PMO::getProcessesByName(e, pids);
    }
    if (pids.empty())
        SKIP("OBR not running");
    REQUIRE((findPatternsExternal(pids.back(), swPattern, false) && !swPattern.empty()));
}

TEST_CASE("7T Optional Test 6 test find code in memory of external process", "[PMO]")
{
    std::vector<DWORD> pids{};
    std::string name;
    for (const auto &e : {OBR_WIN64, OBR_WINGDK})
    {
        name = e;
        PMO::getProcessesByName(name, pids);
    }
    if (pids.empty())
        SKIP("OBR not running");

    REQUIRE((findPatternsExternal(pids.back(), swPattern, false) && !swPattern.empty()));

    HANDLE proc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE,
                              FALSE,
                              pids.back());
    REQUIRE(proc);
    SECTION(name + " Replace code")
    {
        REQUIRE(replaceAllCodeExternal(proc, swPattern));
    }
    CloseHandle(proc);
}

TEST_CASE("07 autogen mask", "[PMO]")
{
    REQUIRE(PMO::Pattern::autoGenerateMask(CRDP_PATTERN) == CRDP_ANSWER);
    REQUIRE(PMO::Pattern::autoGenerateMask(JUMPS_PATTERN) == JUMPS_ANSWER);
    REQUIRE(PMO::Pattern::autoGenerateMask(REX_JUMPS_PATTERN) == REX_JUMPS_ANSWER);
    REQUIRE(PMO::Pattern::autoGenerateMask(OTHER_JUMPS) == OTHER_JUMPS_ANSWER);
}
