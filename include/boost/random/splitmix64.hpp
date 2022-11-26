/* 
 * Copyright David Blackman and Sebastiano Vigna 2018
 * Copyright Matt Borland 2022.
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org for most recent version including documentation.
 * 
 * Use when you need 64 bits of state (e.g. seeding xoshiro256)
 */

#ifndef BOOST_RANDOM_SPLITMIX64_HPP
#define BOOST_RANDOM_SPLITMIX64_HPP

#include <cstdint>
#include <limits>

namespace boost { namespace random {

class splitmix64
{
private:
    std::uint64_t state_;

public:
    splitmix64() : state_ {0xF24B76E3A206C3E6ULL} {}
    explicit splitmix64(std::uint64_t state) : state_ {state} {}

    std::uint64_t next() noexcept
    {
        std::uint64_t z {state_ += 0x9E3779B97F4A7C15ULL};
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;

        return z ^ (z >> 31);
    }

    std::uint64_t operator()() noexcept
    {
        return next();
    }

    constexpr std::uint64_t (max)() const noexcept
    {
        return (std::numeric_limits<std::uint64_t>::max)();
    }

    constexpr std::uint64_t (min)() const noexcept
    {
        return (std::numeric_limits<std::uint64_t>::min)();
    }

    template <typename FIter>
    void fill(FIter first, FIter last)
    {
        while (first != last)
        {
            *first++ = next();
        }
    }

};

}} // Namespace boost::random

#endif // BOOST_RANDOM_SPLITMIX64_HPP
