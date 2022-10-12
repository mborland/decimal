//  Copyright (c) 2022 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MODULE type_traits_test
#include <boost/test/included/unit_test.hpp>

#include "../include/boost/decimal/decimal.hpp"

BOOST_AUTO_TEST_CASE( type_traits )
{
    BOOST_TEST(boost::is_decimal_floating_point_v<boost::decimal::decimal32_t>);
    BOOST_TEST(boost::is_arithmetic<boost::decimal::decimal32_t>::value);
}
