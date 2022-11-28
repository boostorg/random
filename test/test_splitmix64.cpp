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
#include <boost/core/lightweight_test.hpp>
#include <boost/random/splitmix64.hpp>

int main(void)
{
    constexpr auto const_min = (std::numeric_limits<std::uint64_t>::min)();
    constexpr auto const_max = (std::numeric_limits<std::uint64_t>::max)();
    
    boost::random::splitmix64 rng;
    for(int i = 0; i < 1000; ++i) 
    {
        std::uint64_t val = rng();
        BOOST_TEST_GE(val, const_min);
        BOOST_TEST_LE(val, const_max);
    }

    std::array<std::uint64_t, 1000> test_array {};
    rng.generate(test_array.begin(), test_array.end());

    return boost::report_errors();
}
