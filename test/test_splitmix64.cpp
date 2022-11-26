/*
 * Copyright (c) 2010 Steven Watanabe
 * Copyright (c) 2022 Matt Borland
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENCE_1_0.txt)
 *
 */

#include <cstdint>
#include <cassert>
#include <array>
#include <limits>
#include <boost/random/splitmix64.hpp>

// Avoids warning about C-style asserts
#define TEST_ASSERT(x) assert(x)

int main(void)
{
    constexpr auto const_min = (std::numeric_limits<std::uint64_t>::min)();
    constexpr auto const_max = (std::numeric_limits<std::uint64_t>::max)();
    
    boost::random::splitmix64 rng;
    std::uint64_t prev {};
    for(int i = 0; i < 1000; ++i) 
    {
        std::uint64_t val = rng();
        TEST_ASSERT(val > const_min);
        TEST_ASSERT(val < const_max);
        TEST_ASSERT(prev != val);
        prev = val;
    }

    std::array<std::uint64_t, 1000> test_array {};
    rng.fill(test_array.begin(), test_array.end());

    return 0;
}
