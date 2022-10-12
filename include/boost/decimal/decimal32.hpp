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

public:
    decimal32() = default;

    /// 3.2.5  Initialization from coefficient and exponent.
    constexpr decimal32(std::integral auto coeff, int expon);

    /// 3.2.6  Conversion to generic floating-point type
    [[nodiscard]] constexpr float to_float() const noexcept;
    //[[nodiscard]] constexpr double to_double() const noexcept;
    //[[nodiscard]] constexpr long double to_long_double() const noexcept;

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

[[nodiscard]] constexpr float decimal32::to_float() const noexcept
{
    return static_cast<float>(this->mantissa()) * std::pow(10.F, this->expon() - BOOST_DECIMAL32_PRECISION);
}

/// Type alias to match STL
using decimal32_t = decimal32;

} // Namespace boost::decimal

#endif // BOOST_DECIMAL_DECIMAL32_HPP
