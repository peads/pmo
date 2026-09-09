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
#ifdef TEST_OBR
#define SLEEP_WAIT_PATTERN      "\xF3\x0F\x10\x35\xF4\x9B\xCA\x01\xF3\x0F\x58\xC6\xF3\x0F\x11\x05"
#define SLEEP_WAIT_MASK         "xxxx???xxxxxxxxx"
#define SLEEP_WAIT_CODE         "\x0F\x57\xC0\x90\x90\x90\x90\x90\x90\x90\x90\x90"
#define OBR_WIN64               "OblivionRemastered-Win64-Shipping.exe"
#define OBR_WINGDK              "OblivionRemastered-WinGDK-Shipping.exe"
inline PMO::Pattern swPattern{SLEEP_WAIT_PATTERN, SLEEP_WAIT_MASK, SLEEP_WAIT_CODE};
#endif
static const auto idpAddr = reinterpret_cast<uintptr_t>(&IsDebuggerPresent);
static const auto crdpAddr = reinterpret_cast<uintptr_t>(&CheckRemoteDebuggerPresent);
#endif //WMAIN_HPP
