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

#ifndef PARSEJMP_HPP
#define PARSEJMP_HPP
#include <cstdint>

namespace PMO
{
    inline uint64_t parseJmpFarAbsAddr(const uint64_t mask, uint8_t *p)
    {
        return *reinterpret_cast<uint64_t*>(p) & mask;
    }

    inline uint64_t parseJmpFarAbsMod(const uint8_t mod, uint8_t *p)
    {
        // std::cout << std::format("{:b} ", mod);
        switch (mod)
        {
            case 0:         // [m16:32]
                return parseJmpFarAbsAddr(0xFFFF'FFFF, ++p);
            // {
            //     const uint64_t result = parseJmpFarAbsAddr(mask, ++p);
            //     if (result)
            //         std::cout << std::format("{:016X}] ", result);
            //     return result;
            // }
            case 1:         // [rbp+m16:16]
                // mask >>= 24;
            case 2:         // [rbp+m16:32]
            case 3:         // rbp
            default:
                break;
        }
        return false;
    }

    inline uint64_t parseJmpFarAbsRm(const uint8_t rm, uint8_t *p)
    {
        // std::cout << std::format("{:b} ", rm);
        switch (rm)
        {
            case 0:     // [rax], [rax+m16:16], [rax+m16:32], rax
            case 1:     // [rcx], ...
            case 2:     // [rdx], ...
            case 3:     // [rbx], ...
            case 4:     // [sib], ..., rsp
                break;
            case 5:     // [m16:32], [rbp+m16:16], [rbp+m16:32], rbp
                // std::cout << "[";
                return parseJmpFarAbsMod((*p >> 6) & 3, p);
            case 6:     // [rsi], [rsi+m16:16], [rsi+m16:32], rsi
            case 7:     // rdi, ...
            default:
                break;
        }
        return false;
    }

    inline uint64_t parseJmpFarAbs(uint8_t *p)
    {
        // e.g., 0000'04ca'8125'ff48h
        // mod | reg  | r/m
        // 00b | 000b | 000b
        // 25 => 0010 0101
        // r/m := 101b == 25h & 7                == 5
        // reg := 100b == 25h >> 3 & 7 == 4h & 7 == 4
        // mod :=  00b == 25h >> 6 & 3 == 0h & 3 == 0
        return parseJmpFarAbsRm((*reinterpret_cast<uint64_t*>(p) & 0xFF) & 7, p);
    }

    inline uint64_t parseJmp(uint8_t *p)
    {
        switch (*p & 0xFF)
        {
            case 0xEB:  // EB cb jmp short, rip = rip+cb        (cb 8-bit sx to 64-bit)
            case 0xE9:  // E9 cd jmp near rel., rip = rip+cd    (cd 32-bit sx...)
                break;
            case 0xFF:
                // FF r/m64  jmp near abs. ind., rip = r/m64 (r/m64 is offset)
                // FF m16:16 jmp far abs. ind., rip = m16:16 (ModRM)
                // FF m16:32 jmp far abs. ind., rip = m16:32 (ModRM)
                // FF m16:32 jmp far abs. ind., rip = m16:32 ...
                // FF m16:64 jmp far abs. ind., rip = m16:64 ...
                return parseJmpFarAbs(++p);
            default:
                break;
        }

        return false;
    }

    inline uint64_t startParseJmp(uint8_t *p)
    {
        return parseJmp((*p & 0xFF ^ 0x48) ? p : ++p);
    }

    inline uint64_t startParseJmp(const uintptr_t addr)
    {
        return startParseJmp(reinterpret_cast<uint8_t*>(addr));
    }
}
#endif //PARSEJMP_HPP
