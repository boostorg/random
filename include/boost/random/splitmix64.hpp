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
#include <cstdlib>
#include <limits>
#include <random>
#include <array>
#include <string>
#include <ios>
#include <iostream>
#include <type_traits>

namespace boost { namespace random {

class splitmix64
{
private:
    std::uint64_t state_;

    inline std::uint64_t concatenate(std::uint32_t word1, std::uint32_t word2)
    {
        return static_cast<std::uint64_t>(word1) << 32 | word2;
    }

public:
    using result_type = std::uint64_t;
    using seed_type = std::uint64_t;

    static constexpr bool has_fixed_range {false};
    
    void seed(result_type value = 0)
    {
        if (value == 0)
        {
            state_ = UINT64_C(0xA164B43C8F634A13);
        }
        else
        {
            state_ = value;
        }
    }

    template <typename Sseq, typename std::enable_if<!std::is_convertible<Sseq, std::uint64_t>::value, bool>::type = true>
    void seed(Sseq& seq)
    {
        std::array<std::uint32_t, 2> seeds;
        seq.generate(seeds.begin(), seeds.end());

        state_ = concatenate(seeds[0], seeds[1]);
    }

    template <typename T, typename std::enable_if<std::is_convertible<T, std::uint64_t>::value, bool>::type = true>
    void seed(T value = 0)
    {
        seed(static_cast<std::uint64_t>(value));
    }

    explicit splitmix64(std::uint64_t state = 0)
    {
        seed(state);
    }

    template <typename Sseq>
    explicit splitmix64(Sseq& seq)
    {
        seed(seq);
    }

    inline result_type next() noexcept
    {
        std::uint64_t z {state_ += UINT64_C(0x9E3779B97F4A7C15)};
	    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
	    
        return z ^ (z >> 31);
    }

    inline result_type operator()() noexcept
    {
        return next();
    }

    inline void discard(unsigned long long val)
    {
        for (unsigned long long i {}; i < val; ++i)
        {
            next();
        }
    }

    inline friend bool operator==(const splitmix64& lhs, const splitmix64& rhs) noexcept
    {
        if (lhs.state_ == rhs.state_)
        {
            return true;
        }

        return false;
    }

    inline friend bool operator!=(const splitmix64& lhs, const splitmix64& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    template <typename CharT, typename Traits>
    inline friend std::basic_ostream<CharT,Traits>& operator<<(std::basic_ostream<CharT,Traits>& ost, 
                                                               const splitmix64& e)
    {
        std::string sstate = std::to_string(e.state_);
        for (const auto i : sstate)
        {
            ost << i;
        }

        return ost;
    }

    template <typename CharT, typename Traits>
    friend std::basic_istream<CharT,Traits>& operator>>(std::basic_istream<CharT,Traits>& ist, 
                                                        splitmix64& e)
    {
        std::string sstate;
        for (std::size_t i {}; i < std::numeric_limits<std::uint64_t>::digits10; ++i)
        {
            CharT val;
            ist >> val >> std::ws;
            sstate.push_back(val);
        }
        
        e.state_ = std::strtoull(sstate.c_str(), nullptr, 10);

        return ist;
    }

    template <typename FIter>
    inline void generate(FIter first, FIter last)
    {
        while (first != last)
        {
            *first++ = next();
        }
    }

    static constexpr result_type (max)() noexcept
    {
        return (std::numeric_limits<std::uint64_t>::max)();
    }

    static constexpr result_type (min)() noexcept
    {
        return (std::numeric_limits<std::uint64_t>::min)();
    }
};

}} // Namespace boost::random

#endif // BOOST_RANDOM_SPLITMIX64_HPP
