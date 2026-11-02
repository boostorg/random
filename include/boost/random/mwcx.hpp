/*
 * Copyright Sebastiano Vigna 2026.
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org for most recent version including documentation.
 */

#ifndef BOOST_RANDOM_MWCX_HPP
#define BOOST_RANDOM_MWCX_HPP

#include <boost/random/detail/config.hpp>
#include <boost/random/detail/mwcx_arithmetic.hpp>
#include <boost/random/detail/mwcx_base.hpp>
#include <array>
#include <cstdint>

namespace boost {
namespace random {

/**
 * This is MWC192X, a scrambled multiply-with-carry generator with 64-bit
 * digits, lag two, and period approximately 2^191, from "Scrambled
 * Multiply-with-Carry Pseudorandom Number Generators" by Sebastiano Vigna
 * (https://arxiv.org/abs/2610.27313). It is recommended for all purposes:
 * it is one of the fastest generators available and it passes all tests we
 * are aware of. Its only 128-bit operations are a multiplication and a sum,
 * and, since the lag is two, each multiplication uses a digit computed two
 * steps before, so the processor can overlap the multiplications of
 * consecutive steps.
 *
 * The output is scrambled by X: it is the most recent digit xored with the
 * carry, as in David Thomas's MWC64X generator. The underlying generator
 * simulates a multiplicative congruential generator with prime modulus
 * m = 0xff4e24dd418f5433 * 2^128 - 1 and multiplier 2^(-64) mod m.
 *
 * discard() computes the state up to 2^64 - 1 steps ahead in time
 * logarithmic in the number of steps, and jump(c, e) advances the generator
 * by c * 2^e steps, which makes it possible to obtain non-overlapping
 * subsequences at distances larger than discard() can express.
 * jump(1, 96) is equivalent to the jump() function of the reference
 * implementation: it can be used to generate 2^95 non-overlapping
 * subsequences for parallel computations. jump(1, 144) is equivalent to
 * long_jump(): it can be used to generate 2^47 starting points, from
 * each of which jump(1, 96) will generate 2^48 non-overlapping
 * subsequences for parallel distributed computations.
 *
 * The state is formed by two 64-bit digits x and y and a 64-bit carry c, and
 * it is read as the integer x + y * 2^64 + c * 2^128, which must be nonzero
 * and smaller than m. This generator is not suitable for cryptographic
 * purposes.
 */
class mwc192x final : public detail::mwcx_base<mwc192x, std::uint64_t, 3, 3>
{
private:

    using Base = detail::mwcx_base<mwc192x, std::uint64_t, 3, 3>;
    friend Base;

    static constexpr std::uint64_t a = UINT64_C(0xff4e24dd418f5433);

    static words_type modulus() noexcept
    {
        return {{ UINT64_C(0xffffffffffffffff), UINT64_C(0xffffffffffffffff), a - 1 }};
    }

    static words_type inverse_base() noexcept
    {
        return {{ 0, a, 0 }};
    }


public:

    using Base::Base;

    inline result_type next() noexcept
    {
        const std::uint64_t result = state_[1] ^ state_[2];
        std::uint64_t hi;
        const std::uint64_t lo = detail::mwcx_mul_add(a, state_[0], state_[2], hi);
        state_[0] = state_[1];
        state_[1] = lo;
        state_[2] = hi;

        return result;
    }
};

/**
 * This is MWC128X, a scrambled multiply-with-carry generator with 64-bit
 * digits, lag one, and period approximately 2^127, from "Scrambled
 * Multiply-with-Carry Pseudorandom Number Generators" by Sebastiano Vigna
 * (https://arxiv.org/abs/2610.27313). It is an alternative to mwc192x with
 * a smaller state (128 bits), and it passes all tests we are aware of. It is
 * slightly slower than mwc192x, as each multiplication needs the result of
 * the previous step.
 *
 * The output is scrambled by X: it is the most recent digit xored with the
 * carry, as in David Thomas's MWC64X generator. The underlying generator
 * simulates a multiplicative congruential generator with prime modulus
 * m = 0xff6da236274e252e * 2^64 - 1 and multiplier 2^(-64) mod m.
 *
 * discard() computes the state up to 2^64 - 1 steps ahead in time
 * logarithmic in the number of steps, and jump(c, e) advances the generator
 * by c * 2^e steps, which makes it possible to obtain non-overlapping
 * subsequences at distances larger than discard() can express.
 * jump(1, 64) is equivalent to the jump() function of the reference
 * implementation: it can be used to generate 2^63 non-overlapping
 * subsequences for parallel computations. jump(1, 96) is equivalent to
 * long_jump(): it can be used to generate 2^31 starting points, from
 * each of which jump(1, 64) will generate 2^32 non-overlapping
 * subsequences for parallel distributed computations.
 *
 * The state is formed by a 64-bit digit x and a 64-bit carry c, and it is
 * read as the integer x + c * 2^64, which must be nonzero and smaller than m.
 * This generator is not suitable for cryptographic purposes.
 */
class mwc128x final : public detail::mwcx_base<mwc128x, std::uint64_t, 2, 2>
{
private:

    using Base = detail::mwcx_base<mwc128x, std::uint64_t, 2, 2>;
    friend Base;

    static constexpr std::uint64_t a = UINT64_C(0xff6da236274e252e);

    static words_type modulus() noexcept
    {
        return {{ UINT64_C(0xffffffffffffffff), a - 1 }};
    }

    static words_type inverse_base() noexcept
    {
        return {{ a, 0 }};
    }


public:

    using Base::Base;

    inline result_type next() noexcept
    {
        const std::uint64_t result = state_[0] ^ state_[1];
        std::uint64_t hi;
        state_[0] = detail::mwcx_mul_add(a, state_[0], state_[1], hi);
        state_[1] = hi;

        return result;
    }
};

/**
 * This is MWC128X/32, a scrambled multiply-with-carry generator with 32-bit
 * digits, lag three, and period approximately 2^127, from "Scrambled
 * Multiply-with-Carry Pseudorandom Number Generators" by Sebastiano Vigna
 * (https://arxiv.org/abs/2610.27313). It returns 32-bit values, it is
 * recommended for all purposes, and it passes all tests we are aware of. It
 * needs just a 32 x 32 -> 64-bit multiplication.
 *
 * The output is scrambled by X: it is the most recent digit xored with the
 * carry, as in David Thomas's MWC64X generator. The underlying generator
 * simulates a multiplicative congruential generator with prime modulus
 * m = 0xffcccc66 * 2^96 - 1 and multiplier 2^(-32) mod m.
 *
 * discard() computes the state up to 2^64 - 1 steps ahead in time
 * logarithmic in the number of steps, and jump(c, e) advances the generator
 * by c * 2^e steps, which makes it possible to obtain non-overlapping
 * subsequences at distances larger than discard() can express.
 * jump(1, 64) is equivalent to the jump() function of the reference
 * implementation: it can be used to generate 2^63 non-overlapping
 * subsequences for parallel computations. jump(1, 96) is equivalent to
 * long_jump(): it can be used to generate 2^31 starting points, from
 * each of which jump(1, 64) will generate 2^32 non-overlapping
 * subsequences for parallel distributed computations.
 *
 * The state is formed by three 32-bit digits x, y, z and a 32-bit carry c,
 * and it is read as the integer x + y * 2^32 + z * 2^64 + c * 2^96, which
 * must be nonzero and smaller than m. This generator is not suitable for
 * cryptographic purposes.
 */
class mwc128x32 final : public detail::mwcx_base<mwc128x32, std::uint32_t, 4, 2>
{
private:

    using Base = detail::mwcx_base<mwc128x32, std::uint32_t, 4, 2>;
    friend Base;

    static constexpr std::uint32_t a = UINT32_C(0xffcccc66);

    static words_type modulus() noexcept
    {
        return {{ UINT64_C(0xffffffffffffffff), static_cast<std::uint64_t>(a - 1) << 32 | UINT64_C(0xffffffff) }};
    }

    static words_type inverse_base() noexcept
    {
        return {{ 0, a }};
    }


public:

    using Base::Base;

    inline result_type next() noexcept
    {
        const std::uint32_t result = state_[2] ^ state_[3];
        const std::uint64_t t = static_cast<std::uint64_t>(a) * state_[0] + state_[3];
        state_[0] = state_[1];
        state_[1] = state_[2];
        state_[2] = static_cast<std::uint32_t>(t);
        state_[3] = static_cast<std::uint32_t>(t >> 32);

        return result;
    }
};

/**
 * This is MWC64X/32, a scrambled multiply-with-carry generator with 32-bit
 * digits, lag one, and period approximately 2^63, from "Scrambled
 * Multiply-with-Carry Pseudorandom Number Generators" by Sebastiano Vigna
 * (https://arxiv.org/abs/2610.27313). It returns 32-bit values, and it is an
 * alternative to mwc128x32 with a smaller state (64 bits), meant for
 * environments in which registers are scarce, such as GPUs. It needs just a
 * 32 x 32 -> 64-bit multiplication.
 *
 * The output is scrambled by X: it is the most recent digit xored with the
 * carry, as in David Thomas's MWC64X generator. The underlying generator
 * simulates a multiplicative congruential generator with prime modulus
 * m = 0xfff145f8 * 2^32 - 1 and multiplier 2^(-32) mod m.
 *
 * discard() computes the state up to 2^64 - 1 steps ahead in time
 * logarithmic in the number of steps, and jump(c, e) advances the generator
 * by c * 2^e steps, which makes it possible to obtain non-overlapping
 * subsequences at distances larger than discard() can express.
 * jump(1, 32) is equivalent to the jump() function of the reference
 * implementation: it can be used to generate 2^31 non-overlapping
 * subsequences for parallel computations. jump(1, 48) is equivalent to
 * long_jump(): it can be used to generate 2^15 starting points, from
 * each of which jump(1, 32) will generate 2^16 non-overlapping
 * subsequences for parallel distributed computations.
 *
 * The state is formed by a 32-bit digit x and a 32-bit carry c, and it is
 * read as the integer x + c * 2^32, which must be nonzero and smaller than m.
 * This generator is not suitable for cryptographic purposes.
 */
class mwc64x32 final : public detail::mwcx_base<mwc64x32, std::uint32_t, 2, 1>
{
private:

    using Base = detail::mwcx_base<mwc64x32, std::uint32_t, 2, 1>;
    friend Base;

    static constexpr std::uint32_t a = UINT32_C(0xfff145f8);

    static words_type modulus() noexcept
    {
        return {{ (static_cast<std::uint64_t>(a) << 32) - 1 }};
    }

    static words_type inverse_base() noexcept
    {
        return {{ a }};
    }


public:

    using Base::Base;

    inline result_type next() noexcept
    {
        const std::uint32_t result = state_[0] ^ state_[1];
        const std::uint64_t t = static_cast<std::uint64_t>(a) * state_[0] + state_[1];
        state_[0] = static_cast<std::uint32_t>(t);
        state_[1] = static_cast<std::uint32_t>(t >> 32);

        return result;
    }
};

} // namespace random
} // namespace boost

#endif // BOOST_RANDOM_MWCX_HPP
