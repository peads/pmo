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

inline void printDebugger() noexcept
{
    intProducer idp = nullptr;
    PMO::findNamedFunction(reinterpret_cast<uintptr_t>(&IsDebuggerPresent), &idp);
    intBiFunction crdp = nullptr;
    PMO::findNamedFunction(reinterpret_cast<uintptr_t>(&CheckRemoteDebuggerPresent), &crdp);
    for (auto ptr : {reinterpret_cast<uint8_t*>(idp), reinterpret_cast<uint8_t*>(crdp)})
    {
        for (; ptr && *ptr != 0xCC; ++ptr)
            std::cout << std::format("{:02X}", *ptr);
        std::cout << std::endl;
    }
    // std::cout /*<< std::endl << debuggerPatterns[0].mask.str */<< std::endl;
    // for (const auto &e : debuggerPatterns[0].pmsk())
    //     std::cout << std::format("{:016X} ", e.i);
    // std::cout << std::endl;
    // for (const auto &e : debuggerPatterns[0].bmsk())
    //     std::cout << std::format("{:016X} ", e.i);
    std::cout << std::endl << std::endl;
}
inline bool disableDebuggerChecking() noexcept
{
    PMO::Pattern debuggerPatterns[] = {
        PMO::Pattern{IDP_PATTERN, IDP_MASK, IDP_CODE},
        PMO::Pattern{CRDP_PATTERN, CRDP_MASK, CRDP_CODE},
    };
    intProducer idp = nullptr;
    PMO::findNamedFunction(reinterpret_cast<uintptr_t>(&IsDebuggerPresent), &idp);
    const auto idpAddr = reinterpret_cast<uintptr_t>(idp);
    debuggerPatterns[0].push_back(idpAddr);
    const PMO::PointerUnion puI{.address = idpAddr};
    bool result = replaceCode(puI,
                              debuggerPatterns[0].code.str,
                              debuggerPatterns[0].codeLen);

    intBiFunction crdp = nullptr;
    PMO::findNamedFunction(reinterpret_cast<uintptr_t>(&CheckRemoteDebuggerPresent), &crdp);
    const auto crdpAddr = reinterpret_cast<uintptr_t>(crdp);
    const PMO::PointerUnion puC{.address = crdpAddr};
    debuggerPatterns[1].push_back(crdpAddr);
    result &= replaceCode(puC,
                          debuggerPatterns[1].code.str,
                          debuggerPatterns[1].codeLen);

    return result;
}

extern "C" {
    __declspec(dllexport) int DllMain() noexcept
    {
#ifndef IS_DEBUG
        return disableDebuggerChecking();
#else
        printDebugger();
        const auto result = disableDebuggerChecking();
        printDebugger();
        return result;
#endif
    }
}
