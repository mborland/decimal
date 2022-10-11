//  Copyright (c) 2022 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../include/boost/decimal/decimal.hpp"

int main(void)
{
    // Decimal32
    static_assert(boost::is_decimal_floating_point_v<boost::decimal::decimal32_t>);
    static_assert(boost::is_arithmetic<boost::decimal::decimal32_t>::value);
}
