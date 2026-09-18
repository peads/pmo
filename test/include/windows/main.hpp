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

#ifndef WMAIN_HPP
#define WMAIN_HPP
#include "windows/MemoryOps.hpp"
#include "windows/helpers/debug/debug.hpp"
#ifndef DBGPATS
#define DBGPATS
static inline PMO::Pattern debuggerPatterns[] = {
    PMO::Pattern{IDP_PATTERN, IDP_MASK, IDP_CODE},
    PMO::Pattern{CRDP_PATTERN, CRDP_MASK, CRDP_CODE},
};
#endif
#ifdef TEST_OBR
#include "windows/games/mods/qswprime.hpp"
static inline PMO::Pattern swPattern{SLEEP_WAIT_PATTERN, SLEEP_WAIT_MASK, SLEEP_WAIT_CODE};
#endif
#define JUMPS_PATTERN           "\xFF\x15\xF0\xFF\x7F\x00\xFF\x25\xF1\xFF\x7F\x00\xFF\x15\xF2\xFF\x7F\x00\xFF\x25\xF3\xFF\x7F\x00\xFF\x15\xF4\xFF\x7F\x00"
#define REX_JUMPS_PATTERN       "\x48\xFF\x25\xF0\xFF\xFF\x00\x48\xFF\x15\xF1\xFF\xFF\x00\x48\xFF\x25\xF2\xFF\xFF\x00\x48\xFF\x15\xF3\xFF\xFF\x00\x48\xFF\x25\xF4\xFF\xFF\x00\x48\xFF\x15\xF5\xFF\xFF\x00"
#define OTHER_JUMPS             "\xEB\x05\xE9\x00\x10\x00\x00\xFF\xE0\x41\xFF\xE0\xFF\x25\x00\x00\x00\x01\xFF\x28"

#define JUMPS_ANSWER            "xx????xx????xx????xx????xx????"
#define REX_JUMPS_ANSWER        "xxx????xxx????xxx????xxx????xxx????xxx????"
#define OTHER_JUMPS_ANSWER      "x?x????xxxxxxx????xx"
#define DWMAPI_NAME "dwmapi.dll"
static const auto idpAddr = reinterpret_cast<uintptr_t>(&IsDebuggerPresent);
static const auto crdpAddr = reinterpret_cast<uintptr_t>(&CheckRemoteDebuggerPresent);
#endif //WMAIN_HPP
