//  Copyright (c) 2022 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <stdexcept>
#include <concepts>
#include <boost/core/lightweight_test.hpp>

#include "../include/boost/decimal/decimal32.hpp"

template <std::signed_integral T>
void to_signed_type()
{
    boost::decimal::decimal32_t ten(10, 1);
    BOOST_TEST(ten.to<T>() == static_cast<T>(10));

    boost::decimal::decimal32_t neg_ten(-10, 1);
    BOOST_TEST(neg_ten.to<T>() == static_cast<T>(-10));
}

template <std::unsigned_integral T>
void to_unsigned_type()
{
    boost::decimal::decimal32_t ten(10, 1);
    BOOST_TEST(ten.to<T>() == static_cast<T>(10));
}

template <std::integral T>
void throw_test()
{
    boost::decimal::decimal32_t huge(10, 60);
    BOOST_TEST_THROWS(huge.to<T>(), std::overflow_error);
}

int main()
{
    to_signed_type<int>();
    to_signed_type<long>();
    to_signed_type<long long>();

    to_unsigned_type<unsigned>();
    to_unsigned_type<unsigned long>();
    to_unsigned_type<unsigned long long>();

    return boost::report_errors();
}
