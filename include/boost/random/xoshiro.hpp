/* 
 * Copyright Sebastiano Vigna 2019.
 * Copyright David Blackman 2019.
 * Copyright Matt Borland 2022.
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef BOOST_RANDOM_XOSHIRO_HPP
#define BOOST_RANDOM_XOSHIRO_HPP

#include <cstdint>
#include <array>
#include <boost/random/detail/xoshiro_base.hpp>

namespace boost { namespace random {

namespace detail {

template <typename Gen>
std::array<std::uint64_t, 4> four_word_jump_impl(Gen gen, const std::array<std::uint64_t, 4>& vals) noexcept
{
    std::array<std::uint64_t, 4> current_state {gen->state()};
    std::array<std::uint64_t, 4> new_state {};

    for (std::size_t i {}; i < 4U; ++i)
    {
        for (std::size_t j {}; j < 64U; ++j)
        {
            if (vals[i] & static_cast<std::uint64_t>(1) << j)
            {
                for (std::size_t k {}; k < 4U; ++k)
                {
                    new_state[k] ^= current_state[k];
                }
            }

            gen->next();
        }
    }

    return new_state;
}

} // Namespace detail

/*
 * This is xoshiro256++ 1.0, one of our all-purpose, rock-solid generators.
 * It has excellent (sub-ns) speed, a state (256 bits) that is large
 * enough for any parallel application, and it passes all tests we are
 * aware of.
 *
 * For generating just floating-point numbers, xoshiro256+ is even faster.
 */
class xoshiro256_plusplus final : public detail::xoshiro<4>
{
public:
    // Use all of the base classes constructors 
    using detail::xoshiro<4>::xoshiro;
    
    inline result_type next() noexcept override
    {
        const std::uint64_t result = rotl(state_[0] + state_[3], 23) + state_[0];
        const std::uint64_t t = state_[1] << 17;

        state_[2] ^= state_[0];
        state_[3] ^= state_[1];
        state_[1] ^= state_[2];
        state_[0] ^= state_[3];

        state_[2] ^= t;

        state_[3] = rotl(state_[3], 45);

        return result;
    }

    // This is the jump function for the generator. It is equivalent
    // to 2^128 calls to next(); it can be used to generate 2^128
    // non-overlapping subsequences for parallel computations.
    inline void jump() noexcept override
    {
        static constexpr std::array<std::uint64_t, 4> jump_pos {0x180EC6D33CFD0ABA, 0xD5A61266F0C9392C, 
                                                                0xA9582618E03FC9AA, 0x39ABDC4529B1661C};
        state_ = detail::four_word_jump_impl(this, jump_pos);
    }

    // This is the long-jump function for the generator. It is equivalent to
    // 2^192 calls to next(); it can be used to generate 2^64 starting points,
    // from each of which jump() will generate 2^64 non-overlapping
    // subsequences for parallel distributed computations.
    inline void long_jump() noexcept override
    {
        static constexpr std::array<std::uint64_t, 4> long_jump_pos {0x76E15D3EFEFDCBBF, 0xC5004E441C522FB3, 
                                                                     0x77710069854EE241, 0x39109BB02ACBE635};
        state_ = detail::four_word_jump_impl(this, long_jump_pos);
    }
};

/* 
 * This is xoshiro256+ 1.0, our best and fastest generator for floating-point
 * numbers. We suggest to use its upper bits for floating-point
 * generation, as it is slightly faster than xoshiro256++/xoshiro256**. It
 * passes all tests we are aware of except for the lowest three bits,
 * which might fail linearity tests (and just those), so if low linear
 * complexity is not considered an issue (as it is usually the case) it
 * can be used to generate 64-bit outputs, too.
 */
class xoshiro256_plus final : public detail::xoshiro<4>
{
public:
    using detail::xoshiro<4>::xoshiro;

    inline result_type next() noexcept override
    {
        const std::uint64_t result {state_[0] + state_[3]};
        const std::uint64_t t {state_[1] << 17};

        state_[2] ^= state_[0];
        state_[3] ^= state_[1];
        state_[1] ^= state_[2];
        state_[0] ^= state_[3];

        state_[2] ^= t;

        state_[3] = rotl(state_[3], 45);

        return result;
    }

    inline double operator()() noexcept
    {
        return (next() >> 11) * 0x1.0p-53;
    }

    // This is the jump function for the generator. It is equivalent
    // to 2^128 calls to next(); it can be used to generate 2^128
    // non-overlapping subsequences for parallel computations.
    inline void jump() noexcept override
    {
        static constexpr std::array<std::uint64_t, 4> jump_pos {0x180EC6D33CFD0ABA, 0xD5A61266F0C9392C, 
                                                                0xA9582618E03FC9AA, 0x39ABDC4529B1661C};
        state_ = detail::four_word_jump_impl(this, jump_pos);
    }

    // This is the long-jump function for the generator. It is equivalent to
    // 2^192 calls to next(); it can be used to generate 2^64 starting points,
    // from each of which jump() will generate 2^64 non-overlapping
    // subsequences for parallel distributed computations.
    inline void long_jump() noexcept override
    {
        static constexpr std::array<std::uint64_t, 4> long_jump_pos {0x76E15D3EFEFDCBBF, 0xC5004E441C522FB3, 
                                                                     0x77710069854EE241, 0x39109BB02ACBE635};
        state_ = detail::four_word_jump_impl(this, long_jump_pos);
    }
};

/* This is xoshiro256** 1.0, one of our all-purpose, rock-solid
 * generators. It has excellent (sub-ns) speed, a state (256 bits) that is
 * large enough for any parallel application, and it passes all tests we
 * are aware of.
 *
 * For generating just floating-point numbers, xoshiro256+ is even faster.
 */
class xoshiro256_starstar final : public detail::xoshiro<4>
{
public:
    // Use all of the base classes constructors 
    using detail::xoshiro<4>::xoshiro;
    
    inline result_type next() noexcept override
    {
        const std::uint64_t result = rotl(state_[1] * 5, 7) * 9;
        const std::uint64_t t = state_[1] << 17;

        state_[2] ^= state_[0];
        state_[3] ^= state_[1];
        state_[1] ^= state_[2];
        state_[0] ^= state_[3];

        state_[2] ^= t;

        state_[3] = rotl(state_[3], 45);

        return result;
    }

    // This is the jump function for the generator. It is equivalent
    // to 2^128 calls to next(); it can be used to generate 2^128
    // non-overlapping subsequences for parallel computations.
    inline void jump() noexcept override
    {
        static constexpr std::array<std::uint64_t, 4> jump_pos {0x180EC6D33CFD0ABA, 0xD5A61266F0C9392C, 
                                                                0xA9582618E03FC9AA, 0x39ABDC4529B1661C};
        state_ = detail::four_word_jump_impl(this, jump_pos);
    }

    // This is the long-jump function for the generator. It is equivalent to
    // 2^192 calls to next(); it can be used to generate 2^64 starting points,
    // from each of which jump() will generate 2^64 non-overlapping
    // subsequences for parallel distributed computations.
    inline void long_jump() noexcept override
    {
        static constexpr std::array<std::uint64_t, 4> long_jump_pos {0x76E15D3EFEFDCBBF, 0xC5004E441C522FB3, 
                                                                     0x77710069854EE241, 0x39109BB02ACBE635};
        state_ = detail::four_word_jump_impl(this, long_jump_pos);
    }
};

}} // Namespace boost::random

#endif // BOOST_RANDOM_XOSHIRO_HPP
