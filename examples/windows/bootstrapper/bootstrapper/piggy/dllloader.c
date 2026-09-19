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
#include <stdint.h>
#include <stdio.h>
#include <windows.h>

typedef HRESULT (*hresultProducer)();

int main()
{
    static const char *funcs[] = {"DwmFlush", "DllCanUnloadNow"};
    const HMODULE module = LoadLibrary(DLL_NAME);

    if (!module)
        return -1;

    char dllPath[MAX_PATH];
    int result = !GetModuleFileName(module, dllPath, MAX_PATH);

    printf("Bootstrapping dll: %s\n", dllPath);

    for (size_t i = 0; i < 2; ++i)
    {
        // ReSharper disable once CppLocalVariableMayBeConst
        FARPROC addr = GetProcAddress(module, funcs[i]);
        const hresultProducer func = (hresultProducer) addr;
        result |= func();
        printf("%s\n", result != S_OK ? "FAIL" : "S_OK");
    }
    result |= !FreeLibrary(module);
    return result;
}
