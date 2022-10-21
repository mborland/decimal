//  Copyright (c) 2022 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/core/lightweight_test.hpp>

#include "../include/boost/decimal/decimal32.hpp"

int main()
{
    boost::decimal::decimal32 ten {10, 1};
    boost::decimal::decimal32 pos_ten {+ten};
    BOOST_TEST(ten == pos_ten);
    BOOST_TEST(!pos_ten.sign());

    boost::decimal::decimal32 neg_ten {-10, 1};
    BOOST_TEST(ten.sign() != neg_ten.sign());
    BOOST_TEST(neg_ten.sign());

    return boost::report_errors();
}
