//  Copyright (c) 2022 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//  See https://www.open-std.org/JTC1/SC22/WG21/docs/papers/2009/n2849.pdf
//
//  Similar to IEEE 754-2019 but with reduced exponent range for close interop with float
//  and less complex implementation

#ifndef BOOST_DECIMAL_DECIMAL32_HPP
#define BOOST_DECIMAL_DECIMAL32_HPP

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <type_traits>
#include <concepts>
#include <stdexcept>
#include <limits>
#include "tools/config.hpp"

#define BOOST_DECIMAL32_BITS            32
#define BOOST_DECIMAL32_BYTES           4
#define BOOST_DECIMAL32_PRECISION       7
#define BOOST_DECIMAL32_EMAX            63
#define BOOST_DECIMAL32_EMIN            -63
#define BOOST_DECIMAL32_MAN_MAX         9999999
#define BOOST_DECIMAL32_MAN_MIN         1000000

// Use the extra range of the mantissa
#define BOOST_DECIMAL32_INF             0B111111111111111111111111
#define BOOST_DECIMAL32_QUIET_NAN       0B111111111111111111111110
#define BOOST_DECIMAL32_SIGNALING_NAN   0B111111111111111111111101

namespace boost::decimal {

/// 3.2.2 Decimal32
class decimal32 final
{
private:
    struct bit_layout_
    {
        std::uint32_t mantissa : 24;
        std::int32_t  expon    : 7;
        std::uint32_t sign     : 1;
    };

    bit_layout_ data_;

    constexpr void normalize() noexcept;
    
    template <std::floating_point T>
    [[nodiscard]] constexpr T to_floating_point_type() const;

    template <std::integral T>
    [[nodiscard]] constexpr T to_integral_type() const;

public:
    decimal32() = default;

    /// 3.2.5  Initialization from coefficient and exponent.
    constexpr decimal32(std::integral auto coeff, int expon);

    /// 3.2.6  Conversion to generic floating-point type
    [[nodiscard]] constexpr auto to_float() const;
    [[nodiscard]] constexpr auto to_double() const;
    [[nodiscard]] constexpr auto to_long_double() const;

    /// Non-conforming conversion to integral types
    [[nodiscard]] constexpr auto to_int() const;
    [[nodiscard]] constexpr auto to_unsigned_int() const;
    [[nodiscard]] constexpr auto to_long() const;
    [[nodiscard]] constexpr auto to_unsigned_long() const;
    [[nodiscard]] constexpr auto to_long_long() const;
    [[nodiscard]] constexpr auto to_unsigned_long_long() const;

    /// Catch-all templated type
    template <typename T>
        requires std::is_floating_point_v<T> || std::is_integral_v<T>
    [[nodiscard]] constexpr T to() const;

    /// Getters to allow access to the bit layout
    [[nodiscard]] constexpr auto mantissa() const noexcept { return data_.mantissa; }
    [[nodiscard]] constexpr auto expon() const noexcept { return data_.expon; }
    [[nodiscard]] constexpr auto sign() const noexcept { return data_.sign; }

    // TODO: Extra debugging functions. Can be removed for release
    void print() const { std::cout << "Man: " << data_.mantissa << "\nExpon: " << data_.expon << std::endl; }
    constexpr unsigned size() const { return sizeof(data_); }
};

constexpr void decimal32::normalize() noexcept
{
    while (this->mantissa() < BOOST_DECIMAL32_MAN_MIN)
    {
        this->data_.mantissa *= 10;
    }
}

constexpr decimal32::decimal32(std::integral auto coeff, int expon)
{
    this->data_.sign = coeff < 0 ? true : false;
    if (data_.sign)
    {
        coeff = -coeff;
    }

    while (coeff > BOOST_DECIMAL32_MAN_MAX)
    {
        coeff /= 10;
        ++expon;
    }

    if (expon > BOOST_DECIMAL32_EMAX)
    {
        this->data_.mantissa = BOOST_DECIMAL32_INF;
        this->data_.expon = BOOST_DECIMAL32_EMAX;
    }
    else if (expon < BOOST_DECIMAL32_EMIN)
    {
        this->data_.mantissa = BOOST_DECIMAL32_INF;
        this->data_.expon = BOOST_DECIMAL32_EMIN;
    }
    else
    {
        this->data_.mantissa = coeff;
        this->data_.expon = expon;
    }

    this->normalize();
}

template <std::floating_point T>
[[nodiscard]] constexpr T decimal32::to_floating_point_type() const
{
    T temp {static_cast<T>(this->mantissa() * std::pow(static_cast<T>(10.), this->expon() - BOOST_DECIMAL32_PRECISION + 1))};

    // decimal32 can only be larger than floats
    if constexpr (std::is_same_v<T, float>)
    {
        if (temp == HUGE_VALF)
        {
            throw std::overflow_error("Decimal type exceeds the size of the target floating point type");
        }
    }

    if (this->sign())
    {
        temp *= -1;
    }

    return temp;
}

[[nodiscard]] constexpr auto decimal32::to_float() const
{
    return this->to_floating_point_type<float>();
}

[[nodiscard]] constexpr auto decimal32::to_double() const
{
    return this->to_floating_point_type<double>();
}

[[nodiscard]] constexpr auto decimal32::to_long_double() const
{
    return this->to_floating_point_type<long double>();
}

template <std::integral T>
[[nodiscard]] constexpr T decimal32::to_integral_type() const
{
    auto temp {this->to_floating_point_type<double>()};

    if (temp > (std::numeric_limits<T>::max)())
    {
        throw std::overflow_error("Decimal type exceeds the size of the target integer type");
    }

    return static_cast<T>(temp);
}

[[nodiscard]] constexpr auto decimal32::to_int() const
{
    return this->to_integral_type<int>();
}

[[nodiscard]] constexpr auto decimal32::to_unsigned_int() const
{
    return this->to_integral_type<unsigned>();
}

[[nodiscard]] constexpr auto decimal32::to_long() const
{
    return this->to_integral_type<long>();
}

[[nodiscard]] constexpr auto decimal32::to_unsigned_long() const
{
    return this->to_integral_type<unsigned long>();
}

[[nodiscard]] constexpr auto decimal32::to_long_long() const
{
    return this->to_integral_type<long long>();
}

[[nodiscard]] constexpr auto decimal32::to_unsigned_long_long() const
{
    return this->to_integral_type<unsigned long long>();
}

template <typename T>
    requires std::is_floating_point_v<T> || std::is_integral_v<T>
[[nodiscard]] constexpr T decimal32::to() const
{
    if constexpr (std::is_floating_point_v<T>)
    {
        return this->to_floating_point_type<T>();
    }
    else
    {
        return this->to_integral_type<T>();
    }
}

/// Type alias to match STL
using decimal32_t = decimal32;

} // Namespace boost::decimal

#endif // BOOST_DECIMAL_DECIMAL32_HPP
