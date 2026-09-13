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

#ifndef SETWRAPPER_HPP
#define SETWRAPPER_HPP
#include <unordered_set>
#include "types/PseudoContainer.hpp"

namespace PMO
{
    template <typename T>
    concept SetConcept = PseudoContainer<T> && requires(T c, typename T::value_type v)
    {
        c.insert(v);
        c.erase(v);
    };

    template <typename U, SetConcept T = std::unordered_set<U>>
    class SetWrapper : public T
    {
        public:
            void push_back(U u)
            {
                this->insert(u);
            }

            U pop_back()
            {
                auto result = *this->begin();
                this->erase(this->begin());
                return result;
            }

            U pop_front()
            {
                auto result = *this->rbegin();
                this->erase(this->rbegin());
                return result;
            }

            U front()
            {
                return *this->rbegin();
            }

            U back()
            {
                return *this->begin();
            }
    };
}
#endif //SETWRAPPER_HPP
