/* 
 * Copyright Sebastiano Vigna 2019.
 * Copyright David Blackman
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
    static constexpr std::array<std::uint64_t, 4> jump_pos {0x180EC6D33CFD0ABA, 0xD5A61266F0C9392C, 
                                                            0xA9582618E03FC9AA, 0x39ABDC4529B1661C};
public:
    explicit xoshiro256_plusplus(std::uint64_t state = 0)
    {
        seed(state);
    }

    template <typename Sseq, typename std::enable_if<!std::is_convertible<Sseq, xoshiro>::value, bool>::type = true>
    explicit xoshiro256_plusplus(Sseq& seq)
    {
        seed(seq);
    }

    template <typename FIter>
    xoshiro256_plusplus(FIter first, FIter last)
    {
        seed(first, last);
    }

    template <typename... Args, typename Integer = typename std::common_type<Args...>::type,
              typename std::enable_if<std::is_convertible<Integer, std::uint64_t>::value, bool>::type = true>
    xoshiro256_plusplus(Args&& ...args)
    {
        std::initializer_list<Integer> list {std::forward<Args>(args)...};
        seed(list.begin(), list.end());
    }

    xoshiro256_plusplus(const xoshiro256_plusplus& other) = default;
    xoshiro256_plusplus& operator=(const xoshiro256_plusplus& other) = default;
    
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

    void jump() noexcept override
    {

    }

    void long_jump() noexcept override
    {

    }
};

}} // Namespace boost::random

#endif // BOOST_RANDOM_XOSHIRO_HPP
