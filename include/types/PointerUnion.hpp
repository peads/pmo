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
#ifndef POINTERUNION_HPP
#define POINTERUNION_HPP
#include <functional>
namespace PMO
{
    union PointerUnion
    {
        void *ptr = nullptr;
        char *cptr;
        const uintptr_t address;
        const char *str;
        uint64_t *u64ptr;
        uint8_t *u8ptr;

        bool operator<(const PointerUnion &a) const noexcept
        {
            return address < a.address;
        }

        bool operator==(const PointerUnion &a) const noexcept
        {
            return address == a.address;
        }
    };
}

template <>
struct std::hash<PMO::PointerUnion> {
    size_t operator()(const PMO::PointerUnion &a) const noexcept
    {
        return std::hash<uint64_t>{}(a.address);
    }
};
#endif //POINTERUNION_HPP
