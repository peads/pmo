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
#include <windows.h>
#include <wchar.h>
#include <locale.h>
#include <stdio.h>

BOOL WINAPI DllMain(const HMODULE dll, const DWORD reason, LPVOID lpvReserved)
{
    setlocale(LC_ALL, "");

    printf("Piggy-backed dllmain hit\n");
    fprintf(stderr, "Piggy-backed dllmain hit\n");

    wprintf(L"Piggy-backed dllmain hit\n");
    fwprintf(stderr, L"Piggy-backed dllmain hit\n");

    return TRUE;
}
