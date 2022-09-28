//  Copyright (c) 2022 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/decimal/decimal.hpp>
#include <cassert>

int main(void)
{
    boost::decimal::decimal32 dummy {};
    assert(dummy.size() == 4);

    boost::decimal::decimal32_t dummy2(8388608, 2);
    assert(dummy.size() == 4);

    constexpr boost::decimal::decimal32_t constexpr_dummy(10, 2);
    static_assert(dummy.size() == 4);
}
