//  Copyright (c) 2022 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_DECIMAL_DETAIL_TYPE_TRAITS
#define BOOST_DECIMAL_DETAIL_TYPE_TRAITS

#include <type_traits>

#include "../decimal32.hpp"

namespace std {

/// Define new member for decimal floating point
template <typename T>
struct is_decimal_floating_point : public std::false_type {};
template <>
struct is_decimal_floating_point<boost::decimal::decimal32_t> : public std::true_type {};
template <typename T>
constexpr bool is_decimal_floating_point_v = is_decimal_floating_point<T>::value;

/// Specializations of exisitng type traits
template <>
struct is_arithmetic<boost::decimal::decimal32_t> : public std::true_type {};

}

#endif // BOOST_DECIMAL_DETIAL_TYPE_TRAITS
