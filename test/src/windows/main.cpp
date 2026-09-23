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
#include <set>

#include "windows/main.hpp"

#include <catch2/catch_test_macros.hpp>

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
        auto im = PMO::ImportInfo{.name = str};
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
    auto a = *puSet.begin();
    PMO::PointerUnion b{.address = a.address};
    REQUIRE(a == b);
}

// #define pipe0(x) std::views::transform(x, \
//     [](const auto &im) \
//     { \
//         return im.thunks | std::views::values; \
//     }) | std::views::join
// #define pipe1(x) std::views::transform(x, \
//     [](const auto &im) \
//     { \
//         return im.exports | std::views::values | std::views::keys; \
//     }) | std::views::join
// TEST_CASE("000", "[PMO]")
// {
//     PMO::SetWrapper<PMO::ImportInfo> imports{};
//     const auto modules = PMO::getModules<PMO::SetWrapper<HMODULE>>();
//     PMO::findImports(getCurrentModule(), imports);
//     auto imports1 = PMO::getImports<PMO::SetWrapper<PMO::ImportInfo>>();
//
//     PMO::SetWrapper<HMODULE> found;
//     std::ranges::copy(std::views::transform(imports1,
//     [](auto &e)
//     {
//         return PMO::getModule(e.name.c_str());
//     }), std::back_inserter(found));
//
//     for (auto &im : imports)
//     {
//         // std::cout << im.name << std::endl;
//         auto isFound = found.contains(PMO::getModule(im.name.c_str()));
//         if (!isFound)
//             std::cerr << im.name << std::endl;
//         // REQUIRE(isFound);
//     }
//
//     auto pipe0 = std::views::transform(
//     [](const auto &im)FORCE_INLINE_LAMBDA
//     {
//        return im.thunks | std::views::values;
//     }) | std::views::join;
//
//     auto pipe1 = std::views::transform(
//     [](const auto &im)FORCE_INLINE_LAMBDA
//     {
//        return im.exports | std::views::values |
//            std::views::keys;
//     }) | std::views::join;
//
//     REQUIRE(std::ranges::all_of(pipe0(imports),
//     [&imports1, &pipe0](const auto &e)FORCE_INLINE_LAMBDA
//     {
//         return std::ranges::any_of(pipe0(imports1), [&e](const auto &f)FORCE_INLINE_LAMBDA
//         {
//             return f == e;
//         });
//     }));
//
//     REQUIRE(std::ranges::all_of(pipe1(imports),
//     [&imports1, &pipe1](const auto &e)FORCE_INLINE_LAMBDA
//     {
//         return std::ranges::any_of(pipe1(imports1), [&e](const auto &f)FORCE_INLINE_LAMBDA
//         {
//             return f == e;
//         });
//     }));
// }

TEST_CASE("00a more raw search testing", "[PMO]")
{
    reset();

    const std::wstring buf = PMO::getModuleFileName(getCurrentModule());
    const std::filesystem::path path{buf};
    const std::string name = path.filename().string();
    const auto imports = PMO::getImports();

    size_t cnt = 0;
    for (auto &pattern : debuggerPatterns)
    {
        static constexpr size_t expected[] = {1, 1};
        bool foundAtLeastOne = false;
        for (const auto &it : imports)
        {
            auto module = PMO::getModule(it.name.c_str());
            MODULEINFO info = PMO::getModuleInfo(module);
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

    auto module = PMO::getModule("KERNELBASE.dll");
    MODULEINFO info = PMO::getModuleInfo(module);
    REQUIRE(PMO::findPatterns(reinterpret_cast<uintptr_t>(info.lpBaseOfDll), info.
                SizeOfImage, debuggerPatterns[0]));
    REQUIRE(reinterpret_cast<int(*)()>(debuggerPatterns[0].back().address)() == IsDebuggerPresent(
            ));
    auto imports = PMO::getImports();
    bool foundAtLeastOne = false;
    for (const auto &it : imports)
    {
        module = PMO::getModule(it.name.c_str());
        info = PMO::getModuleInfo(module);
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
    auto imports = PMO::getImports();
    for (const auto &im : imports)
    {
        if (!strcmp("KERNEL32.dll", im.name.c_str()))
        {
            REQUIRE(!PMO::findProcByName("IsDebuggerPresent", im.exports).empty());
            REQUIRE(!PMO::findProcByName("CheckRemoteDebuggerPresent", im.exports).empty());
            break;
        }
    }
    imports.clear();
}

TEST_CASE("ZZ Test replace by function name", "[PMO]")
{
    reset();

    int bl[2] = {};

    REQUIRE(CheckRemoteDebuggerPresent(GetCurrentProcess(), bl + 0));
    REQUIRE(IsDebuggerPresent() == bl[0]);

    REQUIRE(disableDebuggerChecking());

    REQUIRE(CheckRemoteDebuggerPresent(GetCurrentProcess(), bl + 1));
    REQUIRE((IsDebuggerPresent() == bl[1] && !bl[1]));

    PMO::Pattern a{IDP_CODE, IDP_MASK, IDP_CODE};
    PMO::Pattern b{CRDP_CODE, CRDP_MASK, CRDP_CODE};

    auto [lpBaseOfDll, SizeOfImage, EntryPoint] =
        PMO::getModuleInfo(PMO::getModule("KERNELBASE.dll"));
    findPatterns(reinterpret_cast<uintptr_t>(lpBaseOfDll), SizeOfImage, a);
    REQUIRE(!a.empty());
    findPatterns(reinterpret_cast<uintptr_t>(lpBaseOfDll), SizeOfImage, b);
    REQUIRE(!b.empty());

}

TEST_CASE("02 Test find by traversing thunks", "[PMO]")
{
    reset();
    auto addr = idpAddr;
    int (*fn)() = nullptr;
    PMO::findNamedFunction(addr, &fn);
    REQUIRE(fn() == IsDebuggerPresent());
    auto module = getCurrentModule();
    REQUIRE(module);

    for (auto imports = PMO::getImports(); auto &im : imports)
    {
        for (const auto &gn : im.thunks | std::views::values)
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
    // char buffer[MAX_PATH];
    // GetModuleFileName(nullptr, buffer, MAX_PATH);
    const std::wstring buffer = PMO::getModuleFileName(getCurrentModule());
    const std::filesystem::path path(buffer);
    REQUIRE((path.has_filename() && "pmo.exe" == path.filename().string()));
    PMO::getProcessesByName(path.filename().string(), handles);
    REQUIRE((!handles.empty() && handles.back() == GetCurrentProcessId()));
}

#ifdef TEST_OBR
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
#endif

TEST_CASE("07 autogen mask", "[PMO]")
{
    reset();

    auto crdpMask = PMO::Pattern::autoGenerateMask(CRDP_PATTERN);
    REQUIRE(crdpMask == CRDP_MASK);
    REQUIRE(PMO::Pattern::autoGenerateMask(JUMPS_PATTERN) == JUMPS_ANSWER);
    REQUIRE(PMO::Pattern::autoGenerateMask(REX_JUMPS_PATTERN) == REX_JUMPS_ANSWER);
    REQUIRE(PMO::Pattern::autoGenerateMask(OTHER_JUMPS) == OTHER_JUMPS_ANSWER);

    const HMODULE module = PMO::getModule("KERNELBASE.dll");
    auto [lpBaseOfDll, SizeOfImage, EntryPoint] = PMO::getModuleInfo(module);
    PMO::Pattern a{
        CRDP_PATTERN,
        sizeof(CRDP_PATTERN) - 1,
        crdpMask.data(),
        crdpMask.length() + 1,
        CRDP_CODE,
        sizeof(CRDP_CODE) - 1
    };

    findPatterns(reinterpret_cast<uintptr_t>(lpBaseOfDll), SizeOfImage, a);
    REQUIRE(!a.empty());

    int b[2] = {};
    int (*fn2)(HMODULE, int *) = *reinterpret_cast<int(*)(HMODULE, int *)>(a.back().address);
    REQUIRE(fn2(module, b + 0) == CheckRemoteDebuggerPresent(module, b + 1));
    REQUIRE(b[0] == b[1]);
}

TEST_CASE("08 Test findExports", "[PMO]")
{
    char systemDir[MAX_PATH];
    GetSystemDirectory(systemDir, MAX_PATH);
    std::filesystem::path path(systemDir);
    path.append(DWMAPI_NAME);
    const HMODULE module = LoadLibrary(path.string().c_str());

    for (std::map<WORD, std::tuple<uintptr_t, char*, bool>> exports{}; const auto &[fn, name,
             isNamed] : exports | std::views::values)
    {
        auto val = GetProcAddress(module, name);
        REQUIRE(fn == reinterpret_cast<uintptr_t>(val));
    }
}
TEST_CASE("00b Test getModuleInfo", "[PMO]")
{
    MODULEINFO info{};
    GetModuleInformation(GetCurrentProcess(), getCurrentModule(), &info, sizeof(MODULEINFO));
    auto [lpBaseOfDll, SizeOfImage, EntryPoint] = PMO::getModuleInfo(getCurrentModule());

    REQUIRE(info.EntryPoint == EntryPoint);
    REQUIRE(info.lpBaseOfDll == lpBaseOfDll);
    REQUIRE(info.SizeOfImage == SizeOfImage);
}
