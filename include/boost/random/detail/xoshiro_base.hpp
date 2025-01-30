/*
 * Copyright Matt Borland 2022 - 2025.
 * Distributed under the Boost Software License, Version 1.0. (See
        * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org for most recent version including documentation.
 *
 * $Id$
 */

#ifndef BOOST_RANDOM_DETAIL_XOSHIRO_BASE
#define BOOST_RANDOM_DETAIL_XOSHIRO_BASE

#include <boost/random/splitmix64.hpp>
#include <boost/throw_exception.hpp>
#include <array>
#include <utility>
#include <stdexcept>
#include <limits>
#include <initializer_list>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <ios>
#include <istream>
#include <type_traits>
#include <iterator>

namespace boost {
namespace random {
namespace detail {

// N is the number of words (e.g. for xoshiro 256 N=4)
template <std::size_t N>
class xoshiro_base
{
protected:

    using xoshiro_type = std::integral_constant<std::size_t, N>;

    std::array<std::uint64_t, N> state_;

    inline std::uint64_t concatenate(std::uint32_t word1, std::uint32_t word2) noexcept
    {
        return static_cast<std::uint64_t>(word1) << 32U | word2;
    }

    inline void jump_impl(const std::integral_constant<std::size_t, 4>&) noexcept
    {
        constexpr std::array<std::uint64_t, 4U> jump = {{ UINT64_C(0x180ec6d33cfd0aba), UINT64_C(0xd5a61266f0c9392c),
                                                          UINT64_C(0xa9582618e03fc9aa), UINT64_C(0x39abdc4529b1661c) }};

        std::uint64_t s0 = 0;
        std::uint64_t s1 = 0;
        std::uint64_t s2 = 0;
        std::uint64_t s3 = 0;

        for (std::uint64_t i = 0; i < jump.size(); i++)
        {
            for (std::size_t b = 0; b < 64U; b++)
            {
                if (jump[i] & UINT64_C(1) << b) {
                    s0 ^= state_[0];
                    s1 ^= state_[1];
                    s2 ^= state_[2];
                    s3 ^= state_[3];
                }

                next();
            }
        }

        state_[0] = s0;
        state_[1] = s1;
        state_[2] = s2;
        state_[3] = s3;
    }

    inline void long_jump_impl(const std::integral_constant<std::size_t, 4>&) noexcept
    {
        constexpr std::array<std::uint64_t, 4> long_jump = {{ UINT64_C(0x76e15d3efefdcbbf), UINT64_C(0xc5004e441c522fb3),
                                                              UINT64_C(0x77710069854ee241), UINT64_C(0x39109bb02acbe635) }};

        std::uint64_t s0 = 0;
        std::uint64_t s1 = 0;
        std::uint64_t s2 = 0;
        std::uint64_t s3 = 0;

        for (std::size_t i = 0; i < long_jump.size(); i++)
        {
            for (std::size_t b = 0; b < 64; b++)
            {
                if (long_jump[i] & UINT64_C(1) << b)
                {
                    s0 ^= state_[0];
                    s1 ^= state_[1];
                    s2 ^= state_[2];
                    s3 ^= state_[3];
                }

                next();
            }
        }

        state_[0] = s0;
        state_[1] = s1;
        state_[2] = s2;
        state_[3] = s3;
    }
    
public:

    using result_type = std::uint64_t;
    using seed_type = std::uint64_t;

    static constexpr bool has_fixed_range {false};

    void seed(const std::uint64_t state = 0)
    {
        splitmix64 gen(state);
        for (auto& i : state_)
        {
            i = gen();
        }
    }

    /**
     * Seeds the generator with 32-bit values produced by @c seq.generate().
     */
    template <typename Sseq,
              typename std::enable_if<!std::is_convertible<Sseq, std::uint64_t>::value, bool>::type = true>
    void seed(Sseq& seq)
    {
        for (auto& i : state_)
        {
            std::array<std::uint32_t, 2> seeds;
            seq.generate(seeds.begin(), seeds.end());

            i = concatenate(seeds[0], seeds[1]);
        }
    }

    /** Sets the state of the generator using values from an iterator range. */
    template <typename FIter>
    void seed(FIter first, FIter last)
    {
        static_assert(std::is_integral<typename std::iterator_traits<FIter>::value_type>::value, "Type must be a built-in integer type" );

        std::size_t offset = 0;
        while (first != last && offset < state_.size())
        {
            state_[offset++] = static_cast<std::uint64_t>(*first++);
        }

        if (offset != state_.size())
        {
            boost::throw_exception(std::invalid_argument("Not enough elements in call to seed."));
        }
    }
     
    /**
     * Seeds the generator with 64-bit values produced by @c seq.generate().
     */
    template <typename Sseq, typename std::enable_if<!std::is_convertible<Sseq, xoshiro_base>::value, bool>::type = true>
    explicit xoshiro_base(Sseq& seq)
    {
        seed(seq);
    }

    /** Seeds the generator with a user provided seed. */
    explicit xoshiro_base(std::uint64_t state = 0)
    {
        seed(state);
    }

    template <typename FIter>
    xoshiro_base(FIter first, FIter last)
    {
        seed(first, last);
    }

    // Hit all of our rule of 5 to ensure old platforms work correctly
    ~xoshiro_base() = default;
    xoshiro_base(const xoshiro_base& other) noexcept { state_ = other.state(); }
    xoshiro_base& operator=(const xoshiro_base& other) noexcept { state_ = other.state(); return *this; }
    xoshiro_base(xoshiro_base&& other) noexcept { state_ = other.state(); }
    xoshiro_base& operator=(xoshiro_base&& other) noexcept { state_ = other.state(); return *this; }

    virtual inline result_type next() noexcept = 0;

    /** This is the jump function for the generator. It is equivalent
     *  to 2^128 calls to next(); it can be used to generate 2^128
     *  non-overlapping subsequences for parallel computations. */
    inline void jump() noexcept
    {
        jump_impl(xoshiro_type());
    }

    /** This is the long-jump function for the generator. It is equivalent to
     *  2^192 calls to next(); it can be used to generate 2^64 starting points,
     *  from each of which jump() will generate 2^64 non-overlapping
     *  subsequences for parallel distributed computations. */
    inline void long_jump() noexcept
    {
        long_jump_impl(xoshiro_type());
    }

    /**  Returns the next value of the generator. */
    inline result_type operator()() noexcept
    {
        return next();
    }

    /** Advances the state of the generator by @c z. */
    inline void discard(const std::uint64_t z) noexcept
    {
        for (std::uint64_t i {}; i < z; ++i)
        {
            next();
        }
    }

    /**
     * Returns true if the two generators will produce identical
     * sequences of values.
     */
    inline friend bool operator==(const xoshiro_base& lhs, const xoshiro_base& rhs) noexcept
    {
        return lhs.state_ == rhs.state_;
    }

    /**
     * Returns true if the two generators will produce different
     * sequences of values.
     */
    inline friend bool operator!=(const xoshiro_base& lhs, const xoshiro_base& rhs) noexcept
    {
        return lhs.state_ != rhs.state_;
    }

    /**  Writes a @c xorshiro to a @c std::ostream. */
    template <typename CharT, typename Traits>
    inline friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& ost,
                                                                const xoshiro_base& e)
    {
        for (std::size_t i {}; i < e.state_.size(); ++i)
        {
            ost << e.state_[i] << ' ';
        }

        return ost;
    }

    /**  Reads a @c xorshiro from a @c std::istream. */
    template <typename CharT, typename Traits>
    inline friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& ist,
                                                                xoshiro_base& e)
    {
        for (std::size_t i {}; i < e.state_.size(); ++i)
        {
            ist >> e.state_[i] >> std::ws;
        }

        return ist;
    }

    /** Fills a range with random values */
    template <typename FIter>
    inline void generate(FIter first, FIter last) noexcept
    {
        while (first != last)
        {
            *first++ = next();
        }
    }

    /**
     * Returns the largest value that the @c xorshiro
     * can produce.
     */
    static constexpr result_type (max)() noexcept
    {
        return (std::numeric_limits<result_type>::max)();
    }

    /**
     * Returns the smallest value that the @c xorshiro
     * can produce.
     */
    static constexpr result_type (min)() noexcept
    {
        return (std::numeric_limits<result_type>::min)();
    }

    inline std::array<std::uint64_t, N> state() const noexcept
    {
        return state_;
    }
};

} // namespace detail
} // namespace random
} // namespace boost

#endif
