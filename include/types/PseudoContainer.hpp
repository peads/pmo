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
#ifndef PSEUDOCONTAINER_HPP
#define PSEUDOCONTAINER_HPP

#include <concepts>

template <typename T>
concept PseudoContainer = requires(T c)
{
    typename T::value_type;
    typename T::iterator;
    typename T::const_iterator;
    typename T::size_type;
    { c.begin() }->std::same_as<typename T::iterator>;
    { c.end() }->std::same_as<typename T::iterator>;
    { c.size() }->std::same_as<typename T::size_type>;
    { c.empty() }->std::convertible_to<bool>;
};
#endif // PSEUDOCONTAINER_HPP
