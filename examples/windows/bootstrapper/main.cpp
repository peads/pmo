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
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows/MemoryOps.hpp>
#include "syscalls.h"
#include <fstream>

#define DEFAULT_DLL_NAME "dwmapi.dll"
#define DEFAULT_DLL_LIST_NAME "dlls.txt"

static struct DllInfo
{
    HMODULE module{};
    DWORD ordinalBase{};
    std::map<WORD, std::tuple<uintptr_t, char*, bool>> exports{};
} dllInfo;

typedef void (*UnicodeBiConsumer)(UNICODE_STRING *, const wchar_t *);
typedef long (*DllQuadFunction)(const wchar_t *, ULONG, UNICODE_STRING *, void *);
typedef void (*UnicodeConsumer)(UNICODE_STRING *);

HMODULE loadLibrary(const char *name)
{
    static const HMODULE ntDll = GetModuleHandle("ntdll.dll");
    if (!ntDll)
        return nullptr;

    static const auto RtlInitUnicodeString = reinterpret_cast<UnicodeBiConsumer>(
        GetProcAddress(ntDll, "RtlInitUnicodeString"));

    static const auto LdrLoadDll = reinterpret_cast<DllQuadFunction>(GetProcAddress(ntDll, "LdrLoadDll"));
    if (!(RtlInitUnicodeString && LdrLoadDll))
        return nullptr;

    const std::string sName(name);
    const std::wstring wsName(sName.begin(), sName.end());

    HANDLE module = nullptr;
    UNICODE_STRING uname;
    RtlInitUnicodeString(&uname, wsName.c_str());

    if (LdrLoadDll(nullptr, 0, &uname, &module) >= 0) // NT_SUCCESS(status)
    {
        // static const UnicodeConsumer RtlFreeUnicodeString = (UnicodeConsumer)GetProcAddress(ntDll, "RtlFreeUnicodeString");
        // if (RtlFreeUnicodeString)
            // RtlFreeUnicodeString(&uname);
        return static_cast<HMODULE>(module);
    }
    return nullptr;
}

static bool populateDllInfo(const std::filesystem::path &name)
{
    dllInfo.module = loadLibrary(name.string().c_str());
    dllInfo.ordinalBase = PMO::findExports(dllInfo.module, dllInfo.exports);
    return !!dllInfo.module;
}

static std::filesystem::path generateSystemDllPath(const char *const name)
{
    char systemDir[MAX_PATH];
    GetSystemDirectory(systemDir, MAX_PATH);
    std::filesystem::path path(systemDir);
    path.append(name);
    return std::move(path);
}

#ifndef IS_BOOTSTRAP
extern "C" uintptr_t *mapping = nullptr;
extern "C" const char dllName[];

static auto generateMapping(const size_t cnt)
{
    mapping = static_cast<uintptr_t*>(calloc(cnt + 1, sizeof(uintptr_t)));
    if (!mapping)
        return false;
    for (const auto &[i, e] :
         std::ranges::enumerate_view(dllInfo.exports | std::views::values | std::views::keys))
    {
        mapping[i] = e;
    }
    return true;
}

bool loadDllsFromFile(const char *const name, std::unordered_map<const char*, HMODULE> &out)
{
    std::ifstream file(name);
    if (!file.is_open())
        return false;

    for (std::string line; std::getline(file, line);)
        if (const HMODULE module = loadLibrary(line.c_str()); module)
            out.emplace(line.c_str(), module);

    file.close();
    return true;
}

BOOL WINAPI DllMain([[maybe_unused]] const HMODULE module, const DWORD reason, LPVOID lpvReserved)
{
    static std::unordered_map<const char*, HMODULE> loadedDlls{};
    switch (reason)
    {
        case DLL_PROCESS_DETACH:
            if (!lpvReserved) // clean up if not proc termination
            {
                free(mapping);
                mapping = nullptr;
                FreeLibrary(dllInfo.module);
                for (auto &dll : loadedDlls | std::views::values)
                {
#ifndef IS_VERBOSE
                    FreeLibrary(dll);
#else
                    if (!FreeLibrary(dll))
                    {
                        std::cerr << GetLastError() << std::endl;
                    }
#endif
                }
            }
            break;
        case DLL_PROCESS_ATTACH:
        {
            if (const auto name = generateSystemDllPath(dllName); !populateDllInfo(name))
                return FALSE;
            generateMapping(dllInfo.exports.size());
            loadDllsFromFile(DEFAULT_DLL_LIST_NAME, loadedDlls);
        }
        break;
        // case DLL_THREAD_DETACH:
        // case DLL_THREAD_ATTACH:
        default:
            break;
    }

    return TRUE;
}
#else
int main(const int argc, char **argv)
{
    const auto fname = argc < 2 ? DEFAULT_DLL_NAME : argv[1];
    const auto fpath = generateSystemDllPath(fname);
    if (!populateDllInfo(fpath))
        return -1;

    const auto fstem = fpath.stem();
    const auto &[module, ordinalBase, exports] = dllInfo;

    std::filesystem::path path(OUT_PATH);
    path.append(fstem.string() + ".asm");
    std::ofstream asmOut(path);
    std::stringstream asmHeader{};
    std::stringstream asmBody{};

    path = path.parent_path().append(fstem.string() + ".def");
    std::ofstream defOut(path);

    asmHeader << "bits 64\nsection .text\nextern mapping\nglobal ";
    asmBody << "\n";
    defOut << std::format("LIBRARY {}\nEXPORTS\n", fstem.string());

    for (const auto &[ord, tup] : exports)
    {
        const auto &[addr, name, isNamed] = tup;
        const auto oord = ord + ordinalBase;
        const auto foord = std::format("f{}", ord);

        asmBody << std::format("{}:\n\tmov rax, [rel mapping]\n\tjmp [rax + {}]\n",
                               foord,
                               ord << 3);
        asmHeader << std::format("{},", foord);

        if (isNamed)
        {
            defOut << std::format("{}=", name);
        }
        else
        {
            defOut << std::format("ordinal{}=", oord);
        }
        defOut << std::format("{} @{}\n", foord, oord);
    }

    asmOut << asmHeader.str() << "dllName\n" << asmBody.str() <<
        std::format("section .rdata\n\tdllName db \"{}\", 0\n", fname);
    defOut << "\n";

    path = path.parent_path().append("bootstrap.txt");
    std::ofstream bootOut(path);
    bootOut << fstem.string() << std::endl;

    bootOut.close();
    asmOut.close();
    defOut.close();
    FreeLibrary(module);
    return 0;
}
#endif
