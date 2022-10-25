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
#include <string>
#include <compare>
#include "tools/config.hpp"

#define BOOST_DECIMAL32_BITS            32
#define BOOST_DECIMAL32_BYTES           4
#define BOOST_DECIMAL32_PRECISION       7
#define BOOST_DECIMAL32_EMAX            63
#define BOOST_DECIMAL32_EMIN            -63
#define BOOST_DECIMAL32_E_BITS          7
#define BOOST_DECIMAL32_MAN_MAX         9999999
#define BOOST_DECIMAL32_MAN_MIN         1000000
#define BOOST_DECIMAL32_MAN_BITS        24

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
        std::uint32_t mantissa : BOOST_DECIMAL32_MAN_BITS;
        std::int32_t  expon    : BOOST_DECIMAL32_E_BITS;
        std::uint32_t sign     : 1;
    };

    bit_layout_ data_;

    constexpr void normalize() noexcept;
    
    template <std::floating_point T>
    [[nodiscard]] constexpr T to_floating_point_type() const;

    template <std::integral T>
    [[nodiscard]] constexpr T to_integral_type() const;

public:
    // Rule of 5
    decimal32() = default;
    decimal32(const decimal32&) = default;
    decimal32(decimal32&&) noexcept = default;
    decimal32& operator=(const decimal32&) = default;
    decimal32& operator=(decimal32&&) = default;
    ~decimal32() = default;

    /// 3.2.5  Initialization from coefficient and exponent.
    constexpr decimal32(std::integral auto coeff, int expon);

    /// Non-standard construct from sign, mantissa, exponent
    constexpr decimal32(bool sign, std::integral auto mantissa, std::integral auto exponent) noexcept;

    /// 3.2.6  Conversion to generic floating-point type
    [[nodiscard]] constexpr auto to_float() const;
    [[nodiscard]] constexpr auto to_double() const;
    [[nodiscard]] constexpr auto to_long_double() const;

    // 3.2.2.5  Conversion to integral type.
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

    /// Non-conforming conversion to string
    [[nodiscard]] inline auto to_string() const;

    // 3.2.7 Unary arithmetic operators
    [[nodiscard]] constexpr decimal32 operator+() noexcept;
    [[nodiscard]] constexpr decimal32 operator-() noexcept;

    // 3.2.8 Binary arithmetic operators
    [[nodiscard]] constexpr decimal32 operator*(decimal32 rhs) const noexcept;
    constexpr void operator*=(decimal32 rhs) noexcept;

    // 3.2.9 Comparison operators
    [[nodiscard]] constexpr bool operator==(decimal32 rhs) noexcept;

    template <std::integral T>
    [[nodiscard]] constexpr bool operator==(T rhs) noexcept;

    [[nodiscard]] constexpr bool operator!=(decimal32 rhs) noexcept;

    template <std::integral T>
    [[nodiscard]] constexpr bool operator!=(T rhs) noexcept;

    [[nodiscard]] constexpr decimal32 operator!() noexcept;

    [[nodiscard]] constexpr bool operator>(decimal32 rhs) const noexcept;

    /// Getters to allow access to the bit layout
    [[nodiscard]] constexpr auto mantissa() const noexcept { return data_.mantissa; }
    [[nodiscard]] constexpr auto exponent() const noexcept { return data_.expon; }
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

constexpr decimal32::decimal32(bool sign, std::integral auto mantissa, std::integral auto exponent) noexcept
{
    this->data_.sign = sign;
    this->data_.mantissa = mantissa;
    this->data_.expon = exponent;

    this->normalize();
}

template <std::floating_point T>
[[nodiscard]] constexpr T decimal32::to_floating_point_type() const
{
    T temp {static_cast<T>(this->mantissa() * std::pow(static_cast<T>(10.), this->exponent() - BOOST_DECIMAL32_PRECISION + 1))};

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

[[nodiscard]] auto decimal32::to_string() const
{
    std::string result {};

    result += std::to_string(this->mantissa());
    result.insert(1, ".");
    result += "e";
    result += std::to_string(this->exponent());

    if (this->sign())
    {
        result.insert(0, "-");
    }

    return result;
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

[[nodiscard]] constexpr decimal32 decimal32::operator+() noexcept
{
    return *this;
}

[[nodiscard]] constexpr decimal32 decimal32::operator-() noexcept
{
    this->data_.sign = !this->data_.sign;
    return *this;
}

[[nodiscard]] constexpr decimal32 decimal32::operator*(decimal32 rhs) const noexcept
{
    return decimal32 {static_cast<bool>(this->sign() * rhs.sign()),
                      this->mantissa() * rhs.mantissa(),
                      this->exponent() + rhs.exponent()};
}

constexpr void decimal32::operator*=(decimal32 rhs) noexcept
{
    *this = *this * rhs;
}

[[nodiscard]] constexpr bool decimal32::operator==(decimal32 rhs) noexcept
{
    if (this->sign() == rhs.sign() &&
        this->exponent() == rhs.exponent() &&
        this->mantissa() == rhs.mantissa())
    {
        return true;
    }

    return false;
}

template <std::integral T>
[[nodiscard]] constexpr bool decimal32::operator==(T rhs) noexcept
{
    if (this->to<T>() == rhs)
    {
        return true;
    }

    return false;
}

[[nodiscard]] constexpr bool decimal32::operator!=(decimal32 rhs) noexcept
{
    return !(*this == rhs);
}

template <std::integral T>
[[nodiscard]] constexpr bool decimal32::operator!=(T rhs) noexcept
{
    return !(*this == rhs);
}

[[nodiscard]] constexpr decimal32 decimal32::operator!() noexcept
{
    auto temp = *this;
    temp.data_.sign = !temp.data_.sign;

    return temp;
}

[[nodiscard]] constexpr bool decimal32::operator>(decimal32 rhs) const noexcept
{
    if (rhs.exponent() < this->exponent())
    {
        return true;
    }
    else if (rhs.exponent() > this->exponent())
    {
        return false;
    }
    
    if(rhs.mantissa() < this->mantissa())
    {
        return true;
    }
    else if (rhs.mantissa() > this->mantissa())
    {
        return false;
    }

    return false;
}

/// Type alias to match STL
using decimal32_t = decimal32;

} // Namespace boost::decimal

// Specializaton of std::numeric_limits
namespace std {

template <>
class numeric_limits<boost::decimal::decimal32>
{
public:
    // Member constants
    static constexpr bool is_specialized {true};
    static constexpr bool is_signed {true};
    static constexpr bool is_integer {false};
    static constexpr bool is_exact {false};

    static constexpr bool has_infinity {true};
    static constexpr bool has_quiet_NaN {true};
    static constexpr bool has_signaling_NaN {true};
    static constexpr auto has_denorm {std::denorm_present};
    static constexpr bool has_denorm_loss {true};

    static constexpr bool round_style {std::round_to_nearest};

    static constexpr bool is_iec559 {false};
    static constexpr bool is_bounded {true};
    static constexpr bool is_modulo {false};

    static constexpr int digits {BOOST_DECIMAL32_MAN_BITS};
    static constexpr int digits_10 {BOOST_DECIMAL32_PRECISION};
    static constexpr int max_digits_10 {BOOST_DECIMAL32_PRECISION};
    
    static constexpr int radix {10};

    static constexpr int min_exponent {BOOST_DECIMAL32_EMIN};
    static constexpr int min_exponent_10 {BOOST_DECIMAL32_EMIN};
    static constexpr int max_exponent {BOOST_DECIMAL32_EMAX};
    static constexpr int max_exponent_10 {BOOST_DECIMAL32_EMAX};
    
    static constexpr bool traps {false};
    static constexpr bool tinyness_before {false};

    // Member functions
    static constexpr boost::decimal::decimal32 min() noexcept {return boost::decimal::decimal32(false, BOOST_DECIMAL32_MAN_MIN, BOOST_DECIMAL32_EMIN);}
    static constexpr boost::decimal::decimal32 lowest() noexcept {return boost::decimal::decimal32(true, BOOST_DECIMAL32_MAN_MAX, BOOST_DECIMAL32_EMAX);}
    static constexpr boost::decimal::decimal32 max() noexcept {return boost::decimal::decimal32(false, BOOST_DECIMAL32_MAN_MAX, BOOST_DECIMAL32_EMAX);}
    static constexpr auto                      epsilon() noexcept {return 0.0000001;}
    static constexpr auto                      round_error() noexcept {return 0.0000005;}
    static constexpr boost::decimal::decimal32 infinity() noexcept {return boost::decimal::decimal32(false, BOOST_DECIMAL32_INF, BOOST_DECIMAL32_EMAX);}
    static constexpr boost::decimal::decimal32 quiet_NaN() noexcept {return boost::decimal::decimal32(true, BOOST_DECIMAL32_QUIET_NAN, BOOST_DECIMAL32_EMAX);}
    static constexpr boost::decimal::decimal32 signaling_NaN() noexcept {return boost::decimal::decimal32(true, BOOST_DECIMAL32_SIGNALING_NAN, BOOST_DECIMAL32_EMAX);}
    static constexpr boost::decimal::decimal32 denorm_min() noexcept {return min();}
};

} // Namespace std

#endif // BOOST_DECIMAL_DECIMAL32_HPP
