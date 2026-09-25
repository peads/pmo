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
#include <cstdint>
#include "debug.hpp"
#include "windows/MemoryOps.hpp"
#ifndef DBGPATS
#define DBGPATS
static PMO::Pattern debuggerPatterns[] = {
    PMO::Pattern{IDP_PATTERN, IDP_MASK, IDP_CODE},
    PMO::Pattern{CRDP_PATTERN, CRDP_MASK, CRDP_CODE},
};
#endif
inline bool disableDebuggerChecking() noexcept
{
    int (*idp)() = nullptr;
    auto addr = reinterpret_cast<uintptr_t>(&IsDebuggerPresent);
    PMO::findNamedFunction(addr, &idp);
    debuggerPatterns[0].push_back(reinterpret_cast<uintptr_t>(idp));
    bool result = replaceCode(debuggerPatterns[0].back(), debuggerPatterns[0].code.str, debuggerPatterns[0].codeLen);

    int (*crdp)(HANDLE, int *) = nullptr;
    addr = reinterpret_cast<uintptr_t>(&CheckRemoteDebuggerPresent);
    PMO::findNamedFunction(addr, &crdp);
    debuggerPatterns[1].push_back(reinterpret_cast<uintptr_t>(crdp));
    result &= replaceCode(debuggerPatterns[1].back(),debuggerPatterns[1].code.str, debuggerPatterns[1].codeLen);

    return result;
}

extern "C" {
    __declspec(dllexport) int DllMain() noexcept {
        return disableDebuggerChecking();
    }
}
