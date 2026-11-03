/*
 * Copyright Sebastiano Vigna 2026.
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org for most recent version including documentation.
 */

#ifndef BOOST_RANDOM_DETAIL_F2X_ARITHMETIC_HPP
#define BOOST_RANDOM_DETAIL_F2X_ARITHMETIC_HPP

#include <array>
#include <cstddef>
#include <cstdint>

namespace boost {
namespace random {
namespace detail {

// Arithmetic in the ring F_2[x] / (p), where p is the characteristic
// polynomial of the transition map of a linear generator over F_2 with
// 64 * W bits of state. Calling next() once is equivalent to multiplying the
// state by x in this ring; thus, advancing the generator by n steps is
// equivalent to multiplying by x^n mod p, and the polynomial x^n mod p can be
// applied to the state with the same accumulate-and-step loop used by the
// predefined jump functions. This is a translation of f2x.c from
// https://prng.di.unimi.it.
//
// Polynomials are arrays of W words; coefficient k is bit k % 64 of word
// k / 64. The characteristic polynomial is monic of degree 64 * W, and only
// its 64 * W low-order coefficients are stored.

// a <- a * x mod p
template <std::size_t W>
inline void f2x_mul_x(std::array<std::uint64_t, W>& a, const std::array<std::uint64_t, W>& p) noexcept
{
    std::uint64_t carry = 0;
    for (std::size_t i = 0; i < W; ++i)
    {
        const std::uint64_t next_carry = a[i] >> 63;
        a[i] = a[i] << 1 | carry;
        carry = next_carry;
    }

    // carry is now the coefficient of x^(64 * W)
    if (carry != 0)
    {
        for (std::size_t i = 0; i < W; ++i)
        {
            a[i] ^= p[i];
        }
    }
}

// Returns a * b mod p, by Horner's rule on the coefficients of a.
template <std::size_t W>
inline std::array<std::uint64_t, W> f2x_mul_mod(const std::array<std::uint64_t, W>& a,
                                                const std::array<std::uint64_t, W>& b,
                                                const std::array<std::uint64_t, W>& p) noexcept
{
    std::array<std::uint64_t, W> r {};
    for (std::size_t k = 64 * W; k-- > 0;)
    {
        f2x_mul_x(r, p);
        if ((a[k / 64] >> (k % 64)) & 1)
        {
            for (std::size_t i = 0; i < W; ++i)
            {
                r[i] ^= b[i];
            }
        }
    }

    return r;
}

// Returns x^(c * 2^e) mod p, by square-and-multiply on the bits of c,
// followed by e squarings.
template <std::size_t W>
inline std::array<std::uint64_t, W> f2x_jump_poly(const std::uint64_t c, const unsigned int e,
                                                  const std::array<std::uint64_t, W>& p) noexcept
{
    std::array<std::uint64_t, W> r {};
    r[0] = 1;

    // Leading zeros of c can be skipped, as they would just square 1
    int k = 63;
    while (k >= 0 && ((c >> k) & 1) == 0)
    {
        --k;
    }

    for (; k >= 0; --k)
    {
        r = f2x_mul_mod(r, r, p);
        if ((c >> k) & 1)
        {
            f2x_mul_x(r, p);
        }
    }

    for (unsigned int i = 0; i < e; ++i)
    {
        r = f2x_mul_mod(r, r, p);
    }

    return r;
}

} // namespace detail
} // namespace random
} // namespace boost

#endif // BOOST_RANDOM_DETAIL_F2X_ARITHMETIC_HPP
