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

#include <fstream>

static std::ofstream logfile("qsw.log");//, std::ios::app);
extern "C" {
    __declspec(dllexport) int DllMain() noexcept
    {
        static PMO::Pattern pattern{SLEEP_WAIT_PATTERN, SLEEP_WAIT_MASK, SLEEP_WAIT_CODE};
        static const char *names[] = {OBR_WIN64, OBR_WINGDK};
        static const HMODULE module = PMO::findModule(names);
        auto [lpBaseOfDll, SizeOfImage, EntryPoint] = PMO::getModuleInfo(module);
        const PMO::PointerUnion pu{lpBaseOfDll};
        logfile << std::format("Module {}\n", lpBaseOfDll);
        auto foundPatterns = findPatterns(pu.address, SizeOfImage, pattern);
        logfile << std::format("Found {} {}\n", foundPatterns, pattern.size());
        bool codeReplaced = false;
        if (foundPatterns)
        {
            codeReplaced = replaceCode(pattern.back(), pattern.code().str,  pattern.codeLen);
            logfile << std::format("Code replaced {}\n", codeReplaced);
        }
        logfile.close();
        return codeReplaced && foundPatterns;
    }
}
