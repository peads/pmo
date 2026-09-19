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
#include "bootstrapper.hpp"
#define DEFAULT_DLL_LIST_NAME "dlls.txt"

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

static bool loadDllsFromFile(
    const char *const name,
    std::filesystem::path &parent,
    std::unordered_map<const char*, HMODULE> &out
)
{
    std::ifstream file(parent.append(name));
    if (!file.is_open())
        return false;

    for (std::string line; std::getline(file, line);)
        if (const HMODULE module = loadLibrary(line.c_str()); module)
            out.emplace(line.c_str(), module);

    file.close();
    return true;
}

extern "C" {
    // ReSharper disable once CppParameterMayBeConst
    BOOL WINAPI DllMain(const HMODULE module, const DWORD reason, LPVOID lpvReserved)
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
                    for (const auto &dll : loadedDlls | std::views::values)
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
                char spath[MAX_PATH];
                std::filesystem::path path{};
                if (GetModuleFileName(module, spath, MAX_PATH))
                {
                    path = std::filesystem::path(spath);
                    path = path.parent_path();
                }
                if (const auto name = generateDllPath(dllName); !populateDllInfo(name))
                    return FALSE;
                generateMapping(dllInfo.exports.size());
                loadDllsFromFile(DEFAULT_DLL_LIST_NAME, path, loadedDlls);
            }
            break;
            // case DLL_THREAD_DETACH:
            // case DLL_THREAD_ATTACH:
            default:
                break;
        }
        return TRUE;
    }
}
