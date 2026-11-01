/*
 * Copyright Sebastiano Vigna 2026.
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Checks the 64 x 64 + 64 -> 128-bit operation of the scrambled
 * multiply-with-carry generators against long multiplication on 16-bit
 * limbs (and against unsigned __int128, if available).
 */

#include <boost/random/detail/mwcx_arithmetic.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstdint>

using std::uint64_t;

// a * x + c by long multiplication on 16-bit limbs
static void reference(const uint64_t a, const uint64_t x, const uint64_t c, uint64_t& lo, uint64_t& hi)
{
    uint64_t r[8] = { 0 };
    for (int i = 0; i < 4; ++i)
    {
        r[i] = (c >> (16 * i)) & 0xffff;
    }

    for (int i = 0; i < 4; ++i)
    {
        uint64_t carry = 0;
        for (int j = 0; j < 4; ++j)
        {
            const uint64_t t = ((a >> (16 * i)) & 0xffff) * ((x >> (16 * j)) & 0xffff) + r[i + j] + carry;
            r[i + j] = t & 0xffff;
            carry = t >> 16;
        }

        for (int k = i + 4; carry != 0; ++k)
        {
            const uint64_t t = r[k] + carry;
            r[k] = t & 0xffff;
            carry = t >> 16;
        }
    }

    lo = r[0] | r[1] << 16 | r[2] << 32 | r[3] << 48;
    hi = r[4] | r[5] << 16 | r[6] << 32 | r[7] << 48;
}

static void check(const uint64_t a, const uint64_t x, const uint64_t c)
{
    uint64_t hi;
    const uint64_t lo = boost::random::detail::mwcx_mul_add(a, x, c, hi);
    uint64_t rlo, rhi;
    reference(a, x, c, rlo, rhi);
    BOOST_TEST_EQ(lo, rlo);
    BOOST_TEST_EQ(hi, rhi);
#if defined(__SIZEOF_INT128__)
    __extension__ typedef unsigned __int128 uint128_type;
    const uint128_type t = static_cast<uint128_type>(a) * x + c;
    BOOST_TEST_EQ(lo, static_cast<uint64_t>(t));
    BOOST_TEST_EQ(hi, static_cast<uint64_t>(t >> 64));
#endif
}

int main()
{
    const uint64_t max = UINT64_C(0xffffffffffffffff);
    const uint64_t edge[] = { 0, 1, 2, UINT64_C(0xffffffff), UINT64_C(0x100000000), UINT64_C(0x8000000000000000), max - 1, max };
    for (const uint64_t a : edge)
    {
        for (const uint64_t x : edge)
        {
            for (const uint64_t c : edge)
            {
                check(a, x, c);
            }
        }
    }

    uint64_t s = UINT64_C(0x9e3779b97f4a7c15);
    for (int i = 0; i < 1000000; ++i)
    {
        s ^= s << 13;
        s ^= s >> 7;
        s ^= s << 17;
        const uint64_t a = s * UINT64_C(0x2545F4914F6CDD1D);
        const uint64_t x = s;
        const uint64_t c = s * UINT64_C(0x9E3779B97F4A7C15);
        check(a, x, c);
        // The multipliers of the generators
        check(UINT64_C(0xff4e24dd418f5433), x, c);
        check(UINT64_C(0xff6da236274e252e), x, c);
    }

    return boost::report_errors();
}
