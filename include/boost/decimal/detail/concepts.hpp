//  Copyright (c) 2022 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_DECIMAL_DETAIL_CONCEPTS
#define BOOST_DECIMAL_DETAIL_CONCEPTS

#include <concepts>
#include "type_traits.hpp"

namespace boost::decimal {

template <typename T>
concept decimal_floating_point = is_decimal_floating_point_v<T>;

} // Namespace boost::decimal

#endif // BOOST_DECIMAL_DETAIL_CONCEPTS
