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
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

int main()
{
    typedef HRESULT (*hresultProducer)();
    typedef struct _ProducerInfo // NOLINT(*-reserved-identifier)
    {
        hresultProducer producer;
        const char *name;
    } ProducerInfo;

    const HMODULE module = GetModuleHandle("dwmapi.dll");
    hresultProducer DllCanUnloadNow = NULL;

    if (module)
    {
        char dllPath[MAX_PATH];
        GetModuleFileName(module, dllPath, MAX_PATH);
        char systemDir[MAX_PATH];
        GetSystemDirectory(systemDir, MAX_PATH);
        printf("Actual dll loaded: %s\nMost likely expected dll due to pragma import: %s\\%s\nheh heh\n",
               dllPath,
               systemDir,
               "dwmapi.dll");

        FARPROC addr = GetProcAddress(module, "DllCanUnloadNow");
        DllCanUnloadNow = (hresultProducer) addr;
    }
    ProducerInfo producers[2] = {
        {.producer = DwmFlush, .name = "DwmFlush"},
    };
    size_t len = sizeof(producers) / sizeof(*producers);
    if (!DllCanUnloadNow)
    {
        --len;
    }
    else
    {
        const ProducerInfo temp = {.producer = DllCanUnloadNow, .name = "DllCanUnloadNow"};
        producers[1] = temp;
    }

    int result = 0;
    for (size_t i = 0; i < len; ++i)
    {
        result |= producers[i].producer();
        printf("%s: %s\n", producers[i].name, result != S_OK ? "FAIL" : "S_OK");
    }
    return result;
}
