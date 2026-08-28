//
// Created by peads on 08/28/2026.
//

#ifndef POINTERUNION_HPP
#define POINTERUNION_HPP
#include <cstdint>
#ifdef _WIN64
#include <windows.h>
#endif
namespace PMO
{
    union PointerUnion
    {
        const void *ptr;
        char *cptr;
        const uintptr_t address;
#ifdef _WIN64
        FARPROC proc;
        HMODULE module;
#endif
    };
}
#endif //POINTERUNION_HPP
