/*
 * Copyright Sebastiano Vigna 2026.
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org for most recent version including documentation.
 */

#ifndef BOOST_RANDOM_DETAIL_MWCX_ARITHMETIC_HPP
#define BOOST_RANDOM_DETAIL_MWCX_ARITHMETIC_HPP

#include <boost/config.hpp>
#include <array>
#include <cstddef>
#include <cstdint>

// The step of the generators with 64-bit digits needs the 128-bit result of
// a * x + c, where a, x, and c are 64-bit words. Defining
// BOOST_RANDOM_MWCX_PORTABLE_MUL forces the portable implementation, which
// is used when the compiler provides neither a 128-bit integer type nor a
// 64 x 64 -> 128-bit multiplication intrinsic (e.g., on 32-bit targets).
#if !defined(BOOST_RANDOM_MWCX_PORTABLE_MUL)
#  if defined(__SIZEOF_INT128__)
#    define BOOST_RANDOM_MWCX_HAS_INT128
#  elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_AMD64)) && !defined(_M_ARM64EC)
#    include <intrin.h>
#    pragma intrinsic(_umul128)
#    define BOOST_RANDOM_MWCX_HAS_UMUL128
#  elif defined(_MSC_VER) && (defined(_M_ARM64) || defined(_M_ARM64EC))
#    include <intrin.h>
#    define BOOST_RANDOM_MWCX_HAS_UMULH
#  endif
#endif

namespace boost {
namespace random {
namespace detail {

/**
 * Returns the lower 64 bits of a * x + c, and stores the upper 64 bits in hi.
 *
 * The sum cannot overflow 128 bits, as (2^64 - 1)^2 + 2^64 - 1 < 2^128.
 */
inline std::uint64_t mwcx_mul_add(const std::uint64_t a, const std::uint64_t x, const std::uint64_t c, std::uint64_t& hi) noexcept
{
#if defined(BOOST_RANDOM_MWCX_HAS_INT128)
    __extension__ typedef unsigned __int128 uint128_type;
    const uint128_type t = static_cast<uint128_type>(a) * x + c;
    hi = static_cast<std::uint64_t>(t >> 64);
    return static_cast<std::uint64_t>(t);
#elif defined(BOOST_RANDOM_MWCX_HAS_UMUL128)
    std::uint64_t h;
    const std::uint64_t l = _umul128(a, x, &h) + c;
    hi = h + (l < c);
    return l;
#elif defined(BOOST_RANDOM_MWCX_HAS_UMULH)
    const std::uint64_t l = a * x + c;
    hi = __umulh(a, x) + (l < c);
    return l;
#else
    // Schoolbook multiplication on 32-bit halves. The two halves of c are
    // added to the first two partial products: each partial product can
    // absorb two 32-bit addends without overflow, as
    // (2^32 - 1)^2 + 2 * (2^32 - 1) = 2^64 - 1.
    const std::uint64_t mask = UINT64_C(0xffffffff);
    const std::uint64_t a0 = a & mask, a1 = a >> 32, x0 = x & mask, x1 = x >> 32;
    const std::uint64_t t0 = a0 * x0 + (c & mask);
    const std::uint64_t t1 = a0 * x1 + (t0 >> 32) + (c >> 32);
    const std::uint64_t t2 = a1 * x0 + (t1 & mask);
    hi = a1 * x1 + (t1 >> 32) + (t2 >> 32);
    return (t2 << 32) | (t0 & mask);
#endif
}

// Minimal modular arithmetic on multiple-precision natural numbers, used by
// discard() and jump(). Numbers are little-endian arrays of 64-bit words, and
// operands are assumed to be smaller than the modulus. Multiplication is
// shift-and-add, which needs no wide multiplication: it is slow, but jumps
// are rare, and at these sizes a proper multiple-precision library would not
// be much faster.

/** Returns whether a >= b. */
template <std::size_t N>
inline bool mwcx_ge(const std::array<std::uint64_t, N>& a, const std::array<std::uint64_t, N>& b) noexcept
{
    for (std::size_t i = N; i-- != 0;)
    {
        if (a[i] != b[i])
        {
            return a[i] > b[i];
        }
    }

    return true;
}

/** Returns whether 0 < s < m, that is, whether s is a valid state of an MWC generator with modulus m. */
template <std::size_t N>
inline bool mwcx_is_valid(const std::array<std::uint64_t, N>& s, const std::array<std::uint64_t, N>& m) noexcept
{
    bool nonzero = false;
    for (std::size_t i = 0; i < N; ++i)
    {
        nonzero |= s[i] != 0;
    }

    return nonzero && !mwcx_ge(s, m);
}

/** Returns the number of bits of the little-endian integer of n words held in x (zero for zero). */
inline std::size_t mwcx_bit_len(const std::uint64_t* x, std::size_t n) noexcept
{
    while (n != 0 && x[n - 1] == 0)
    {
        --n;
    }

    if (n == 0)
    {
        return 0;
    }

    std::size_t bits = 64 * n;
    for (std::uint64_t w = x[n - 1]; (w & (UINT64_C(1) << 63)) == 0; w <<= 1)
    {
        --bits;
    }

    return bits;
}

/** Sets a to (a + b) mod m. */
template <std::size_t N>
inline void mwcx_add_mod(std::array<std::uint64_t, N>& a, const std::array<std::uint64_t, N>& b, const std::array<std::uint64_t, N>& m) noexcept
{
    std::uint64_t carry = 0;
    for (std::size_t i = 0; i < N; ++i)
    {
        const std::uint64_t s = a[i] + b[i];
        const std::uint64_t c0 = s < a[i];
        a[i] = s + carry;
        carry = c0 | (a[i] < s);
    }

    // The sum is smaller than 2m, so one subtraction suffices; if the sum
    // overflowed, the subtraction wraps around to the correct result.
    if (carry != 0 || mwcx_ge(a, m))
    {
        std::uint64_t borrow = 0;
        for (std::size_t i = 0; i < N; ++i)
        {
            const std::uint64_t d = a[i] - m[i];
            const std::uint64_t b0 = d > a[i];
            a[i] = d - borrow;
            borrow = b0 | (a[i] > d);
        }
    }
}

/** Returns a * b mod m. */
template <std::size_t N>
inline std::array<std::uint64_t, N> mwcx_mul_mod(const std::array<std::uint64_t, N>& a, const std::array<std::uint64_t, N>& b, const std::array<std::uint64_t, N>& m) noexcept
{
    std::array<std::uint64_t, N> r {};
    std::array<std::uint64_t, N> t = a;
    const std::size_t bits = mwcx_bit_len(b.data(), N);
    for (std::size_t i = 0; i < bits; ++i)
    {
        if ((b[i / 64] >> (i % 64)) & 1)
        {
            mwcx_add_mod(r, t, m);
        }

        const std::array<std::uint64_t, N> u = t;
        mwcx_add_mod(t, u, m);
    }

    return r;
}

/** Returns base^n mod m, where n is the little-endian integer of len words held in exp. */
template <std::size_t N>
inline std::array<std::uint64_t, N> mwcx_pow_mod(const std::array<std::uint64_t, N>& base, const std::uint64_t* exp, const std::size_t len, const std::array<std::uint64_t, N>& m) noexcept
{
    std::array<std::uint64_t, N> r {};
    r[0] = 1;
    std::array<std::uint64_t, N> b = base;
    // Right-to-left square-and-multiply
    const std::size_t bits = mwcx_bit_len(exp, len);
    for (std::size_t i = 0; i < bits; ++i)
    {
        if ((exp[i / 64] >> (i % 64)) & 1)
        {
            r = mwcx_mul_mod(r, b, m);
        }

        if (i + 1 < bits)
        {
            b = mwcx_mul_mod(b, b, m);
        }
    }

    return r;
}

} // namespace detail
} // namespace random
} // namespace boost

#endif // BOOST_RANDOM_DETAIL_MWCX_ARITHMETIC_HPP
