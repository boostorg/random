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

class xoshiro256_plusplus final : public detail::xoshiro<4>
{
private:
    void jump_impl(const std::array<std::uint64_t, 4>& vals) noexcept
    {
        std::array<std::uint64_t, 4> new_state {};

        for (std::size_t i {}; i < 4U; ++i)
        {
            for (std::size_t j {}; j < 64U; ++j)
            {
                if (vals[i] & static_cast<std::uint64_t>(1) << j)
                {
                    for (std::size_t k {}; k < 4U; ++k)
                    {
                        new_state[k] ^= state_[k];
                    }
                }

                next();
            }
        }

        state_ = new_state;
    }

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
        jump_impl(jump_pos);
    }

    // This is the long-jump function for the generator. It is equivalent to
    // 2^192 calls to next(); it can be used to generate 2^64 starting points,
    // from each of which jump() will generate 2^64 non-overlapping
    // subsequences for parallel distributed computations.
    inline void long_jump() noexcept override
    {
        static constexpr std::array<std::uint64_t, 4> long_jump_pos {0x76E15D3EFEFDCBBF, 0xC5004E441C522FB3, 
                                                                     0x77710069854EE241, 0x39109BB02ACBE635};
        jump_impl(long_jump_pos);
    }
};

}} // Namespace boost::random

#endif // BOOST_RANDOM_XOSHIRO_HPP
