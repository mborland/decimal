//  Copyright (c) 2022 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/core/lightweight_test.hpp>
#include <stdexcept>

#include "../include/boost/decimal/decimal32.hpp"

template <typename T>
void to_type()
{
    boost::decimal::decimal32_t ten(10, 1);
    BOOST_TEST(ten.to_float() == static_cast<T>(10));

    boost::decimal::decimal32_t neg_ten(-10, 1);
    BOOST_TEST(neg_ten.to_float() == static_cast<T>(-10));
}

int main()
{
    to_type<float>();
    to_type<double>();
    to_type<long double>();

    boost::decimal::decimal32_t huge(10, 60);
    BOOST_TEST_THROWS(huge.to_float(), std::overflow_error);

    return boost::report_errors();
}
