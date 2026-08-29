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
#ifndef PSEUDOVECTOR_HPP
#define PSEUDOVECTOR_HPP
#include <vector>
#include "types/PseudoContainer.hpp"
namespace PMO
{
    template <typename T>
    concept PseudoVectorConcept = PseudoContainer<T> && requires(T c, typename T::value_type v)
    {
        { c.data() }->std::same_as<typename T::value_type*>;
    };

    // template <PseudoVectorConcept T = std::vector, typename U = typename T::value_type>
    template <typename U, PseudoVectorConcept T = std::vector<U>>
    class PseudoVector : public T
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
#endif //PSEUDOVECTOR_HPP
