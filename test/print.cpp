//  Copyright (c) 2022 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../include/boost/decimal/decimal32.hpp"

int main (void)
{
    boost::decimal::decimal32_t dummy(9999999, 2);
    dummy.print();

    return 0;
}
