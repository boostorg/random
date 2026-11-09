/*
 * Copyright Sebastiano Vigna 2026.
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org for most recent version including documentation.
 */

#ifndef BOOST_RANDOM_DETAIL_MWCX_BASE_HPP
#define BOOST_RANDOM_DETAIL_MWCX_BASE_HPP

#include <boost/random/detail/mwcx_arithmetic.hpp>
#include <boost/throw_exception.hpp>
#include <boost/config.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <istream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace boost {
namespace random {
namespace detail {

/**
 * Common base of the scrambled multiply-with-carry generators.
 *
 * Digit is the type of the digits (std::uint32_t or std::uint64_t), K is the
 * number of words of the state (the digits and the carry), and N is the
 * number of 64-bit words of the state. The state is stored in state_, least
 * significant word first: the oldest digit is state_[0], and the carry is
 * state_[K - 1]. Read as an integer s, the state is an element of the
 * multiplicative group of the integers modulo the prime m = a * b^(K - 1) - 1,
 * where b = 2^w is the base and w is the size in bits of a digit; each step
 * multiplies s by b^(-1) mod m.
 *
 * Derived must provide next() and the static functions modulus() (m) and
 * inverse_base() (b^(-1) mod m), returning std::array<std::uint64_t, N>.
 */
template <typename Derived, typename Digit, std::size_t K, std::size_t N>
class mwcx_base
{
    static_assert(std::is_same<Digit, std::uint32_t>::value || std::is_same<Digit, std::uint64_t>::value,
                  "The digits must be 32-bit or 64-bit unsigned integers");
    static_assert(K * sizeof(Digit) == N * sizeof(std::uint64_t), "Inconsistent state size");

protected:

    using words_type = std::array<std::uint64_t, N>;

    std::array<Digit, K> state_;

private:

    // Below this distance, discard() steps the generator instead of jumping.
    // The cost of a jump grows quadratically with the size of the state, and
    // linearly with the logarithm of the distance; this threshold is close to
    // the break-even point on current x86-64 processors.
    static constexpr std::uint64_t discard_threshold = UINT64_C(1024) << (2 * N);

    static words_type to_words(const std::array<Digit, K>& s, const std::true_type&) noexcept
    {
        words_type w;
        for (std::size_t i = 0; i < N; ++i)
        {
            w[i] = s[i];
        }

        return w;
    }

    static words_type to_words(const std::array<Digit, K>& s, const std::false_type&) noexcept
    {
        words_type w;
        for (std::size_t i = 0; i < N; ++i)
        {
            w[i] = static_cast<std::uint64_t>(s[2 * i]) | static_cast<std::uint64_t>(s[2 * i + 1]) << 32;
        }

        return w;
    }

    static std::array<Digit, K> from_words(const words_type& w, const std::true_type&) noexcept
    {
        std::array<Digit, K> s;
        for (std::size_t i = 0; i < K; ++i)
        {
            s[i] = static_cast<Digit>(w[i]);
        }

        return s;
    }

    static std::array<Digit, K> from_words(const words_type& w, const std::false_type&) noexcept
    {
        std::array<Digit, K> s;
        for (std::size_t i = 0; i < N; ++i)
        {
            s[2 * i] = static_cast<Digit>(w[i]);
            s[2 * i + 1] = static_cast<Digit>(w[i] >> 32);
        }

        return s;
    }

    using is_64 = std::integral_constant<bool, sizeof(Digit) == sizeof(std::uint64_t)>;

    words_type words() const noexcept
    {
        return to_words(state_, is_64());
    }

    void set_words(const words_type& w) noexcept
    {
        state_ = from_words(w, is_64());
    }

    // Sets the state to s; if s is not a valid state, the carry is set to one.
    void set_state(const words_type& w) noexcept
    {
        set_words(w);
        if (!mwcx_is_valid(w, Derived::modulus()))
        {
            state_[K - 1] = 1;
        }
    }

    // Multiplies the state, read as an integer, by r modulo m.
    void jump_mul(const words_type& r) noexcept
    {
        set_words(mwcx_mul_mod(words(), r, Derived::modulus()));
    }

    // The upper half of the product of x and y.
    static std::uint32_t mul_hi(const std::uint32_t x, const std::uint32_t y) noexcept
    {
        return static_cast<std::uint32_t>(static_cast<std::uint64_t>(x) * y >> 32);
    }

    static std::uint64_t mul_hi(const std::uint64_t x, const std::uint64_t y) noexcept
    {
        std::uint64_t hi;
        mwcx_mul_add(x, y, 0, hi);
        return hi;
    }

    static std::uint64_t splitmix(std::uint64_t& x) noexcept
    {
        std::uint64_t z = (x += UINT64_C(0x9E3779B97F4A7C15));
        z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
        z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
        return z ^ (z >> 31);
    }

public:

    using result_type = Digit;
    using seed_type = std::uint64_t;

    // Required for old Boost.Random concept
    static constexpr bool has_fixed_range {false};

    /**
     * Seeds the generator with a user provided seed, as suggested by the
     * reference implementation: the digits and the carry are initialized to
     * valid values using the output of a SplitMix generator seeded with
     * @c value (32-bit values take the lower and then the upper half of each
     * output).
     */
    void seed(const seed_type value) noexcept
    {
        std::uint64_t x = value;
        std::uint64_t z = 0;
        for (std::size_t i = 0; i < K; ++i)
        {
            BOOST_IF_CONSTEXPR (sizeof(Digit) == sizeof(std::uint64_t))
            {
                state_[i] = static_cast<Digit>(splitmix(x));
            }
            else
            {
                if (i % 2 == 0)
                {
                    z = splitmix(x);
                }

                state_[i] = static_cast<Digit>(z >> (32 * (i % 2)));
            }
        }

        // The value r used for the carry is read as a fixed-point fraction
        // r / 2^w in [0..1) and multiplied by a - 2, the size of the range of
        // valid carries: the carry is 1 + (r * (a - 2) >> w), where w is the
        // size in bits of a digit.
        state_[K - 1] = 1 + mul_hi(state_[K - 1], Derived::a - 2);
    }

    /** Seeds the generator with the default seed (zero). */
    void seed() noexcept
    {
        seed(static_cast<seed_type>(0));
    }

    /**
     * Initializes the digits and the carry to valid values using 32-bit
     * values produced by @c seq.generate().
     */
    template <typename Sseq, typename std::enable_if<!std::is_convertible<Sseq, seed_type>::value, bool>::type = true>
    void seed(Sseq& seq)
    {
        std::array<std::uint32_t, 2 * N> seeds;
        seq.generate(seeds.begin(), seeds.end());

        words_type w;
        for (std::size_t i = 0; i < N; ++i)
        {
            w[i] = static_cast<std::uint64_t>(seeds[2 * i]) | static_cast<std::uint64_t>(seeds[2 * i + 1]) << 32;
        }

        set_words(w);
        // The value used for the carry is mapped into the range of valid
        // carries as in seed(value).
        state_[K - 1] = 1 + mul_hi(state_[K - 1], Derived::a - 2);
    }

    /**
     * Sets the state of the generator using values from an iterator range:
     * the digits, oldest first, and then the carry. If the resulting state is
     * not valid, that is, if it is zero or not smaller than the modulus, the
     * carry is set to one.
     */
    template <typename FIter>
    void seed(FIter first, FIter last)
    {
        static_assert(std::is_integral<typename std::iterator_traits<FIter>::value_type>::value,
                      "Value type must be a built-in integer type" );

        std::array<Digit, K> s;
        std::size_t offset = 0;
        while (first != last && offset < K)
        {
            s[offset++] = static_cast<Digit>(*first++);
        }

        if (offset != K)
        {
            boost::throw_exception(std::invalid_argument("Not enough elements in call to seed."));
        }

        // The values are an explicit state: unlike in seed(value) and
        // seed(seq), the carry is not mapped into the range of valid carries.
        set_state(to_words(s, is_64()));
    }

    /** Constructs a generator and calls @c seed(). */
    mwcx_base() noexcept { seed(); }

    /** Seeds the generator with a user provided seed. */
    explicit mwcx_base(const seed_type value) noexcept
    {
        seed(value);
    }

    /** Sets the state of the generator using values from an iterator range. */
    template <typename FIter>
    mwcx_base(FIter& first, FIter last) { seed(first, last); }

    /**
     * Sets the state of the generator using 32-bit values produced by @c seq.generate().
     *
     * @xmlnote
     * The copy constructor will always be preferred over
     * the templated constructor.
     * @endxmlnote
     */
    template <typename Sseq, typename std::enable_if<!std::is_convertible<Sseq, mwcx_base>::value, bool>::type = true>
    explicit mwcx_base(Sseq& seq)
    {
        seed(seq);
    }

    inline result_type next() noexcept
    {
        return static_cast<Derived*>(this)->next();
    }

    /** Returns the next value of the generator. */
    inline result_type operator()() noexcept
    {
        return next();
    }

    /**
     * Advances the state of the generator by @c z steps. Large distances are
     * computed in time logarithmic in @c z, by multiplying the state by
     * b^(-z) mod m.
     */
    inline void discard(const std::uint64_t z) noexcept
    {
        if (z < discard_threshold)
        {
            for (std::uint64_t i = 0; i < z; ++i)
            {
                next();
            }
        }
        else
        {
            jump(z, 0);
        }
    }

    /**
     * Advances the state of the generator by c * 2^e steps, in time
     * logarithmic in c plus linear in e, by multiplying the state by
     * b^(-c * 2^e) mod m. This makes it possible to obtain non-overlapping
     * subsequences at distances larger than discard() can express. For the
     * jump to be meaningful, c * 2^e should be smaller than the period of the
     * generator.
     */
    inline void jump(const std::uint64_t c, const unsigned int e) noexcept
    {
        words_type r = mwcx_pow_mod(Derived::inverse_base(), &c, 1, Derived::modulus());
        for (unsigned int i = 0; i < e; ++i)
        {
            r = mwcx_mul_mod(r, r, Derived::modulus());
        }

        jump_mul(r);
    }

    /**
     * Returns true if the two generators will produce identical
     * sequences of values.
     */
    inline friend bool operator==(const mwcx_base& lhs, const mwcx_base& rhs) noexcept
    {
        return lhs.state_ == rhs.state_;
    }

    /**
     * Returns true if the two generators will produce different
     * sequences of values.
     */
    inline friend bool operator!=(const mwcx_base& lhs, const mwcx_base& rhs) noexcept
    {
        return lhs.state_ != rhs.state_;
    }

    /** Writes the state of the generator to a @c std::ostream. */
    template <typename CharT, typename Traits>
    inline friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& ost,
                                                                const mwcx_base& e)
    {
        for (std::size_t i = 0; i < K; ++i)
        {
            ost << e.state_[i] << ' ';
        }

        return ost;
    }

    /** Reads the state of the generator from a @c std::istream. */
    template <typename CharT, typename Traits>
    inline friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& ist,
                                                                mwcx_base& e)
    {
        for (std::size_t i = 0; i < K; ++i)
        {
            ist >> e.state_[i] >> std::ws;
        }

        return ist;
    }

    /** Fills a range with random values */
    template <typename FIter>
    inline void generate(FIter first, FIter last) noexcept
    {
        using iter_type = typename std::iterator_traits<FIter>::value_type;

        while (first != last)
        {
            *first++ = static_cast<iter_type>(next());
        }
    }

    /** Returns the largest value that the generator can produce. */
    static constexpr result_type (max)() noexcept
    {
        return (std::numeric_limits<result_type>::max)();
    }

    /** Returns the smallest value that the generator can produce. */
    static constexpr result_type (min)() noexcept
    {
        return (std::numeric_limits<result_type>::min)();
    }

    /** Returns the state of the generator: the digits, oldest first, and then the carry. */
    inline std::array<Digit, K> state() const noexcept
    {
        return state_;
    }
};

} // namespace detail
} // namespace random
} // namespace boost

#endif // BOOST_RANDOM_DETAIL_MWCX_BASE_HPP
