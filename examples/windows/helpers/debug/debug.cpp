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

#include <thread>

#include "windows/MemoryOps.hpp"

inline bool disableDebuggerChecking(const HMODULE inModule) noexcept
{
    PMO::Pattern idpPattern{IDP_PATTERN, IDP_MASK, IDP_CODE};
    PMO::Pattern crdpPattern{CRDP_PATTERN, CRDP_MASK, CRDP_CODE};
    
    intProducer idp = nullptr;
    PMO::findNamedFunction(reinterpret_cast<uintptr_t>(&IsDebuggerPresent), &idp);
    const auto idpAddr = reinterpret_cast<uintptr_t>(idp);
    idpPattern.push_back(idpAddr);
    const PMO::PointerUnion puI{.address = idpAddr};
    bool result = replaceCode(puI, idpPattern.code().str, idpPattern.codeLen);

    intBiFunction crdp = nullptr;
    PMO::findNamedFunction(reinterpret_cast<uintptr_t>(&CheckRemoteDebuggerPresent), &crdp);
    const auto crdpAddr = reinterpret_cast<uintptr_t>(crdp);
    const PMO::PointerUnion puC{.address = crdpAddr};
    crdpPattern.push_back(crdpAddr);
    result &= replaceCode(puC, crdpPattern.code().str, crdpPattern.codeLen);
    if (inModule)
        FreeLibraryAndExitThread(inModule, !result);
    return result;
}

extern "C" {
    BOOL WINAPI DllMain(const HMODULE module, const DWORD reason, [[maybe_unused]] LPVOID lpvReserved) noexcept
    {
        if (DLL_PROCESS_ATTACH == reason)
        {
            std::jthread(disableDebuggerChecking, module).detach();
        }
        return TRUE;
    }
}
