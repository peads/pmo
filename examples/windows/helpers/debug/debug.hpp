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

#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "types/pattern/PatternImpl.hpp"

// IsDebuggerPresent uses the pattern below
#define IDP_PATTERN             "\x65\x48\x8B\x04\x25\x60\x00\x00\x00\x0F\xB6\x40\x02\xC3"
#define IDP_MASK                "xxxxxxxxxxxxxx"
#define IDP_CODE                "\x31\xC0\xC3\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90"
// CheckDebuggerPresent uses the pattern below
#define CRDP_PATTERN            "\x48\x8B\xC4\x48\x89\x58\x10\x57\x48\x83\xEC\x30\x33\xDB\x48\x8B\xFA\x48\x89\x58\x08\x48\x85\xC9\x74\x3F\x48\x85\xD2\x74\x3A\x44\x8D\x4B\x08\x48\x89\x58\xE8\x4C\x8D\x40\x08\x8D\x53\x07\x48\xFF\x15\xA3\x99\x19\x00\x0F\x1F\x44\x00\x00\x85\xC0\x78\x30\x48\x39\x5C\x24\x40\xB8\x01\x00\x00\x00\x0F\x95\xC3\x89\x1F\x48\x8B\x5C\x24\x48\x48\x83\xC4\x30\x5F\xC3"
#define CRDP_MASK               "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx????xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define CRDP_CODE               "\xB8\x01\x00\x00\x00\x31\xD2\xC3"

static PMO::Pattern debuggerPatterns[] = {
    PMO::Pattern{IDP_PATTERN, IDP_MASK, IDP_CODE},
    PMO::Pattern{CRDP_PATTERN, CRDP_MASK, CRDP_CODE},
};

bool disableDebuggerChecking() noexcept;
#endif //DEBUG_HPP
