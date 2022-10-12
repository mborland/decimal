//  Copyright (c) 2022 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MODULE size_test
#include <boost/test/included/unit_test.hpp>

#include "../include/boost/decimal/decimal32.hpp"

BOOST_AUTO_TEST_CASE( size_comp )
{
    boost::decimal::decimal32_t default_constructor {};
    BOOST_TEST(default_constructor.size() == 4);

    boost::decimal::decimal32_t size_constructor(4,2);
    BOOST_TEST(size_constructor.size() == 4);
}

BOOST_AUTO_TEST_CASE( constexpr_comp )
{
    constexpr boost::decimal::decimal32_t constexpr_test(4,2);
    static_assert(constexpr_test.size() == 4);
}
