//  Copyright (c) 2022 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//  Specializes boost maths special functions for decimal types
//  if using standalone mode provide them

#ifndef BOOST_DECIMAL_DETAIL_MATH_HPP
#define BOOST_DECIMAL_DETAIL_MATH_HPP

#include "concepts.hpp"

#ifndef BOOST_DECIMAL_STANDALONE

#include <boost/math/special_functions/sign.hpp>

namespace boost::math {

template <>
int signbit<boost::decimal::decimal32>(boost::decimal::decimal32 x)
{
    return static_cast<int>(x.sign());
}

template <>
inline boost::decimal::decimal32 copysign<boost::decimal::decimal32>(const boost::decimal::decimal32& mag, const boost::decimal::decimal32& sgn)
{
    return boost::decimal::decimal32 {static_cast<bool>(sgn.sign()), mag.mantissa(), mag.exponent()};
}

} // namespace boost::math

#else // Standalone math functions rather than template specializations

namespace boost::math {

[[nodiscard]] constexpr bool signbit(boost::decimal::decimal_floating_point auto x) noexcept
{
    return static_cast<bool>(x.sign());
}

template <boost::decimal::decimal_floating_point T>
[[nodiscard]] constexpr T copysign(T mag, T sgn) noexcept
{
    return T {static_cast<bool>(sgn.sign()), mag.mantissa(), mag.exponent()};
}

} // Namespace boost::math

#endif // BOOST_DECIMAL_STANDALONE

#endif // BOOST_DECIMAL_DETAIL_MATH_HPP
