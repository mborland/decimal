//  Copyright (c) 2022 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <concepts>
#include <stdexcept>
#include <string>
#include <boost/core/lightweight_test.hpp>

#include "../include/boost/decimal/decimal32.hpp"

int main()
{
    boost::decimal::decimal32_t ten(10, 1);
    std::string string_ten {"1.000000e1"};
    BOOST_TEST(ten.to_string() == string_ten);

    boost::decimal::decimal32_t neg_ten(-10, 1);
    std::string string_neg_ten {"-1.000000e1"};
    BOOST_TEST(neg_ten.to_string() == string_neg_ten);

    return boost::report_errors();
}
