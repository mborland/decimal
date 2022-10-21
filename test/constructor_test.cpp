//  Copyright (c) 2022 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MODULE constructor_test
#include <boost/test/included/unit_test.hpp>

#include "../include/boost/decimal/decimal32.hpp"

BOOST_AUTO_TEST_CASE( int_constructor )
{
    boost::decimal::decimal32_t ten(10, 1);
    BOOST_TEST(ten.mantissa() == BOOST_DECIMAL32_MAN_MIN);
    BOOST_TEST(ten.exponent() == 1);
    BOOST_TEST(!ten.sign());
}
