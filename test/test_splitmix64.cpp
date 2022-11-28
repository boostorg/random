/*
 * Copyright (c) 2010 Steven Watanabe
 * Copyright (c) 2022 Matt Borland
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENCE_1_0.txt)
 *
 */

#include <cstdint>
#include <array>
#include <limits>
#include <random>
#include <boost/core/lightweight_test.hpp>
#include <boost/math/statistics/univariate_statistics.hpp>
#include <boost/random/splitmix64.hpp>

int main(void)
{
    // Test the values are in the specified min and max range
    constexpr auto const_min = (std::numeric_limits<std::uint64_t>::min)();
    constexpr auto const_max = (std::numeric_limits<std::uint64_t>::max)();
    boost::random::splitmix64 rng;
    for(int i = 0; i < 1000; ++i) 
    {
        std::uint64_t val = rng();
        BOOST_TEST_GE(val, const_min);
        BOOST_TEST_LE(val, const_max);
    }

    // Test the generate function
    std::array<std::uint64_t, 1000> test_array {};
    rng.generate(test_array.begin(), test_array.end());

    // Value validation after 10000 uses with specified seed a la STL
    // Seed = 42
    // N = 10000
    // Validation value = 0x62ed6a69aa8c7b8d
    boost::random::splitmix64 validation {UINT64_C(42)};
    for(int i = 0; i < 9999; ++i)
    {
        validation();
    }

    const std::uint64_t validation_value = validation();
    BOOST_TEST_EQ(validation_value, UINT64_C(0x62ed6a69aa8c7b8d));

    // Test use in a standard distribution
    // Mean of uniform int distribution should be 5 given (1, 9)
    std::uniform_int_distribution<std::uint64_t> distrib(1,9);
    std::array<std::uint64_t, 1000> values {};
    for(std::size_t i = 0; i < 1000; ++i)
    {
        values[i] = distrib(rng);
    }

    const auto dist_mean = boost::math::statistics::mean(values);
    BOOST_TEST_GE(dist_mean, 4.5);
    BOOST_TEST_LE(dist_mean, 5.5);

    // Test that two default constructed generators do not have the same state
    boost::random::splitmix64 gen1;
    boost::random::splitmix64 gen2;
    BOOST_TEST_NE(gen1(), gen2());

    // Test that two specifed generators do have the same state
    boost::random::splitmix64 gen3 {UINT64_C(42)};
    boost::random::splitmix64 gen4 {UINT64_C(42)};
    BOOST_TEST_EQ(gen3(), gen4());

    return boost::report_errors();
}
