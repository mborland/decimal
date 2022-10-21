//  Copyright (c) 2022 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//  Specializes boost maths special functions for decimal types
//  if using standalone mode provide them

#ifndef BOOST_DECIMAL_DETAIL_MATH_HPP
#define BOOST_DECIMAL_DETAIL_MATH_HPP

#ifndef BOOST_DECIMAL_STANDALONE

#include "concepts.hpp"

namespace boost::math {

template <>
int signbit<boost::decimal::decimal32>(boost::decimal::decimal32 x)
{
    return static_cast<int>(x.sign());
}

} // namespace boost::math

#else // Standalone math functions rather than template specializations

namespace boost::math {

[[nodiscard]] constexpr bool signbit(boost::decimal::decimal_floating_point auto x) noexcept
{
    return static_cast<bool>(x.sign());
}

} // Namespace boost::math

#endif // BOOST_DECIMAL_STANDALONE

#endif // BOOST_DECIMAL_DETAIL_MATH_HPP
