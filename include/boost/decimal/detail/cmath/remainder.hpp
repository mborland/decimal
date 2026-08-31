// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_REMAINDER_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_REMAINDER_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/cmath/fmod.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <type_traits>
#include <limits>
#include <cmath>
#endif

namespace boost {
namespace decimal {

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto remainder(const T x, const T y) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    constexpr T zero {0, 0};
    constexpr T half {5, -1};

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isinf(x) || (abs(y) == zero && !isnan(x)))
    {
        return std::numeric_limits<T>::quiet_NaN();
    }
    else if (isnan(x))
    {
        return x;
    }
    else if (isnan(y))
    {
        return y;
    }
    else if (isinf(y))
    {
        return x;
    }
    #else
    if (abs(y) == zero)
    {
        return zero;
    }
    #endif

    constexpr T two {2, 0};

    // Build on the truncated remainder rather than on a rounded quotient. Forming
    // x - round(x/y) * y needs digits(quotient) + digits(y) of precision to come out
    // exact, which the format does not have; fmod carries the operands in a wider
    // integer and is exact. From there the answer is at most one y away.
    const auto abs_y {abs(y)};
    const auto truncated {fmod(x, y)};
    const auto abs_truncated {abs(truncated)};
    const auto half_abs_y {abs_y * half};

    if (abs_truncated < half_abs_y)
    {
        return truncated;
    }

    if (abs_truncated == half_abs_y)
    {
        // A halfway quotient rounds to even, so it only moves when truncation left it odd.
        // x = n*y + truncated with n = 2k + p, so fmod(x, 2y) is truncated when n is even
        // and y + truncated when it is odd.
        const auto doubled {two * abs_y};

        if (!isfinite(doubled) || fmod(x, doubled) == truncated)
        {
            return truncated;
        }
    }

    return truncated - copysign(abs_y, truncated);
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_REMAINDER_HPP
