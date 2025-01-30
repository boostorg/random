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

#ifndef BOOST_RANDOM_XOSHIRO_HPP
#define BOOST_RANDOM_XOSHIRO_HPP

#include <boost/random/detail/xoshiro_base.hpp>
#include <boost/core/bit.hpp>
#include <array>
#include <cstdint>

namespace boost {
namespace random {

/**
 * This is xoshiro256++ 1.0, one of our all-purpose, rock-solid generators.
 * It has excellent (sub-ns) speed, a state (256 bits) that is large
 * enough for any parallel application, and it passes all tests we are
 * aware of.
 *
 * For generating just floating-point numbers, xoshiro256+ is even faster.
 */
class xoshiro256_plusplus final : public detail::xoshiro_base<xoshiro256_plusplus, 4>
{
private:

    friend class detail::xoshiro_base<xoshiro256_plusplus, 4>;

public:

    inline result_type next() noexcept
    {
        const std::uint64_t result = boost::core::rotl(state_[0] + state_[3], 23) + state_[0];
        const std::uint64_t t = state_[1] << 17;

        state_[2] ^= state_[0];
        state_[3] ^= state_[1];
        state_[1] ^= state_[2];
        state_[0] ^= state_[3];

        state_[2] ^= t;

        state_[3] = boost::core::rotl(state_[3], 45);

        return result;
    }
};

/**
 * This is xoshiro256+ 1.0, our best and fastest generator for floating-point
 * numbers. We suggest to use its upper bits for floating-point
 * generation, as it is slightly faster than xoshiro256++/xoshiro256**. It
 * passes all tests we are aware of except for the lowest three bits,
 * which might fail linearity tests (and just those), so if low linear
 * complexity is not considered an issue (as it is usually the case) it
 * can be used to generate 64-bit outputs, too.
 */

class xoshiro256_plus final : public detail::xoshiro_base<xoshiro256_plus, 4>
{
private:

    friend class detail::xoshiro_base<xoshiro256_plus, 4>;

public:

    inline result_type next() noexcept
    {
        const std::uint64_t result = state_[0] + state_[3];
        const std::uint64_t t = state_[1] << 17;

        state_[2] ^= state_[0];
        state_[3] ^= state_[1];
        state_[1] ^= state_[2];
        state_[0] ^= state_[3];

        state_[2] ^= t;

        state_[3] = boost::core::rotl(state_[3], 45);

        return result;
    }

    inline double next_double() noexcept
    {
        #if (__cplusplus >= 201703L || _MSVC_LANG >= 201703L) && defined(__cpp_hex_float) && __cpp_hex_float >= 201603L
        return (next() >> 11) * 0x1.0p-53;
        #else
        return (next() >> 11) * 1.11022302462515654e-16;
        #endif
    }
};

/**
 * This is xoshiro256** 1.0, one of our all-purpose, rock-solid
 * generators. It has excellent (sub-ns) speed, a state (256 bits) that is
 * large enough for any parallel application, and it passes all tests we
 * are aware of.
 *
 * For generating just floating-point numbers, xoshiro256+ is even faster.
 */

class xoshiro256_starstar final : public detail::xoshiro_base<xoshiro256_starstar, 4>
{
private:

    friend class detail::xoshiro_base<xoshiro256_starstar, 4>;

public:

    inline result_type next() noexcept
    {
        const std::uint64_t result = boost::core::rotl(state_[1] * 5, 7) * 9U;
        const std::uint64_t t = state_[1] << 17;

        state_[2] ^= state_[0];
        state_[3] ^= state_[1];
        state_[1] ^= state_[2];
        state_[0] ^= state_[3];

        state_[2] ^= t;

        state_[3] = boost::core::rotl(state_[3], 45);

        return result;
    }
};

} // namespace random
} // namespace boost

#endif //BOOST_RANDOM_XOSHIRO_HPP
