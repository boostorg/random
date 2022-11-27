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

namespace boost { namespace random {

class splitmix64
{
private:
    std::uint64_t state_;

public:
    using result_type = std::uint64_t;
    
    splitmix64() : state_ {0xF24B76E3A206C3E6ULL} {}
    explicit splitmix64(std::uint64_t state) : state_ {state} {}
    
    // Copying is explicity deleted to avoid two generators with the same state by mistake
    splitmix64(const splitmix64&) = delete;
    splitmix64 operator=(const splitmix64&) = delete;

    std::uint64_t next() noexcept
    {
        std::uint64_t z {state_ += UINT64_C(0x9E3779B97F4A7C15)};
	    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
	    
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

    // From https://en.cppreference.com/w/cpp/numeric/random/random_device/entropy
    // This function is not fully implemented in some standard libraries. 
    // For example, LLVM libc++ prior to version 12 always returns zero even though the device is non-deterministic. 
    // In comparison, Microsoft Visual C++ implementation always returns 32, and boost.random returns 10.
    // The entropy of the Linux kernel device /dev/urandom may be obtained using ioctl RNDGETENTCNT - 
    // that's what std::random_device::entropy() in GNU libstdc++ uses as of version 8.1
    constexpr double entropy() const noexcept
    {
        return 10.0;
    }

    template <typename FIter>
    void generate(FIter first, FIter last)
    {
        while (first != last)
        {
            *first++ = next();
        }
    }
};

}} // Namespace boost::random

#endif // BOOST_RANDOM_SPLITMIX64_HPP
