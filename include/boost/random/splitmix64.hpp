/* 
 * Copyright Sebastiano Vigna 2015.
 * Copyright Matt Borland 2022.
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 * 
 *  This is a fixed-increment version of Java 8's SplittableRandom generator
 *  See http://dx.doi.org/10.1145/2714064.2660195 and
 *  http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html
 *  It is a very fast generator passing BigCrush, and it can be useful if
 *  for some reason you absolutely want 64 bits of state; otherwise, we
 *  rather suggest to use a xoroshiro128+ (for moderately parallel
 *  computations) or xorshift1024* (for massively parallel computations)
 *  generator.
 */

#ifndef BOOST_RANDOM_SPLITMIX64_HPP
#define BOOST_RANDOM_SPLITMIX64_HPP

#include <cstdint>
#include <limits>
#include <random>

namespace boost { namespace random {

class splitmix64
{
private:
    std::uint64_t state_;

public:
    using result_type = std::uint64_t;
    
    splitmix64(std::uint64_t state = 0)
    {
        if (state == 0)
        {
            // std::random_device provides a 32-bit result. Concatenate two for the initial 64-bit state
            std::random_device rng;
            std::uint32_t word1 {rng()};
            std::uint32_t word2 {rng()};

            state_ = static_cast<std::uint64_t>(word1) << 32 | word2;
        }
        else
        {
            state_ = state;
        }
    }
    
    // Copying is explicity deleted to avoid two generators with the same state by mistake
    splitmix64(const splitmix64&) = delete;
    splitmix64 operator=(const splitmix64&) = delete;

    inline std::uint64_t next() noexcept
    {
        std::uint64_t z {state_ += UINT64_C(0x9E3779B97F4A7C15)};
	    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
	    
        return z ^ (z >> 31);
    }

    inline std::uint64_t operator()() noexcept
    {
        return next();
    }

    static constexpr std::uint64_t (max)() noexcept
    {
        return (std::numeric_limits<std::uint64_t>::max)();
    }

    static constexpr std::uint64_t (min)() noexcept
    {
        return (std::numeric_limits<std::uint64_t>::min)();
    }
};

}} // Namespace boost::random

#endif // BOOST_RANDOM_SPLITMIX64_HPP
