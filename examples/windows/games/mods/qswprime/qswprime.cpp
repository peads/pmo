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

#include "windows/MemoryOps.hpp"
#include "qswprime.hpp"

#include <thread>

// #ifdef IS_DEBUG
#include <fstream>
#include <format>
// #endif

extern "C" {
    void apply(const HMODULE inModule)
    {
// #ifdef IS_DEBUG
        std::ofstream out("qsw.log", std::ios::app);
//#endif
        PMO::Pattern pattern{SLEEP_WAIT_PATTERN, SLEEP_WAIT_MASK, SLEEP_WAIT_CODE};
        const char *names[] = {OBR_WIN64, OBR_WINGDK};
        const HMODULE module = PMO::findModule(names);
        auto [lpBaseOfDll, SizeOfImage, EntryPoint] = PMO::getModuleInfo(module);
        const PMO::PointerUnion pu{lpBaseOfDll};
// #ifdef IS_DEBUG
        out << std::hex << module << " " << lpBaseOfDll << " " << SizeOfImage << " " << EntryPoint << std::endl;
// #endif
        const bool result =
            findPatterns(pu.address, SizeOfImage, pattern, true/*, PMO::searchBytewise*/) &&
                replaceCode(pattern.back(), pattern.code().str, pattern.codeLen);
// #ifdef IS_DEBUG
        out << "Size: " << pattern.size() << "Initial result: " << std::boolalpha << result << std::endl;
        if (!result)
        {
            out << "Falling back to bytewise search.\n";
            if (!findPatterns(pu.address, SizeOfImage, pattern, true, PMO::searchBytewise))
                out << "Bytewise search failed.\n";
            else if (!replaceCode(pattern.back(), pattern.code().str, pattern.codeLen))
                out << "Failed to replace code.\n";
        }
        for (const auto &e : pattern)
        {
            for (size_t i = 0; i < pattern.codeLen; ++i)
                out << std::format("{:02X} ", reinterpret_cast<char*>(e)[i]);
            out << std::endl;
        }
        out.close();
// #endif
        FreeLibraryAndExitThread(inModule, !result);
    }

    BOOL WINAPI DllMain(const HMODULE module, const DWORD reason, LPVOID lpvReserved) noexcept
    {
        if (DLL_PROCESS_ATTACH == reason)
        {
            std::jthread(apply, module).detach();
        }
        return TRUE;
    }
}
