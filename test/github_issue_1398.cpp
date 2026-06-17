// Copyright 2026 Chris Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// See: https://github.com/boostorg/decimal/issues/1398

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <limits>

template <typename T>
void test()
{
    using namespace boost::decimal;

    constexpr auto builtin_neg_inf = -std::numeric_limits<T>::infinity();

    const auto d32 = decimal32_t(builtin_neg_inf);
    BOOST_TEST(isinf(d32));
    BOOST_TEST(d32 < 0);

    const auto d64 = decimal64_t(builtin_neg_inf);
    BOOST_TEST(isinf(d64));
    BOOST_TEST(d64 < 0);

    const auto d128 = decimal128_t(builtin_neg_inf);
    BOOST_TEST(isinf(d128));
    BOOST_TEST(d128 < 0);
}

int main()
{
    test<float>();
    test<double>();

    return boost::report_errors();
}
