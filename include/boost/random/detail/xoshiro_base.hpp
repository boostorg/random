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

namespace boost {
namespace random {
namespace detail {

// N is the number of words (e.g. for xoshiro 256 N=4)
template <std::size_t N>
class xoshiro_base
{
protected:

    std::array<std::uint64_t, N> state_;

    inline std::uint64_t concatenate(std::uint32_t word1, std::uint32_t word2) noexcept
    {
        return static_cast<std::uint64_t>(word1) << 32U | word2;
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

    template <typename... Args, typename Integer = typename std::common_type<Args...>::type,
              typename std::enable_if<std::is_convertible<Integer, std::uint64_t>::value, bool>::type = true>
    xoshiro_base(Args&& ...args)
    {
        std::initializer_list<Integer> list {std::forward<Args>(args)...};
        seed(list.begin(), list.end());
    }

    xoshiro_base(const xoshiro_base& other) = default;
    xoshiro_base& operator=(const xoshiro_base& other) = default;

    // The following functions are pure virtual as each generator has its own method
    virtual inline result_type next() noexcept = 0;
    virtual inline void jump() noexcept = 0;
    virtual inline void long_jump() noexcept = 0;

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
