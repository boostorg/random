/*
 * Copyright Sebastiano Vigna 2026.
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * This file copies and pastes the original code for comparison under the following license
 *
 * Written in 2023-2026 by Sebastiano Vigna (vigna@acm.org)
 *
 * To the extent possible under law, the author has dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "test_comp_mwcx.hpp"
#include <cstdint>

using boost::random::mwc192x;
using std::uint64_t;

#if defined(__SIZEOF_INT128__)

/* The reference implementation (https://github.com/vigna/MWCPRNG/blob/main/prngs/MWC192X.c),
   with the state in a structure. */

#define MWC_A2 0xff4e24dd418f5433

struct reference
{
    uint64_t x, y, c;

    uint64_t next(void) {
        const uint64_t result = y ^ c;
        const __uint128_t t = MWC_A2 * (__uint128_t)x + c;
        x = y;
        y = t;
        c = t >> 64;
        return result;
    }
};

static void compare_with_reference(const uint64_t x, const uint64_t y, const uint64_t c)
{
    reference r = { x, y, c };
    mwc192x e = make_engine<mwc192x>({ x, y, c });
    // The index of the first output that differs from the reference implementation
    int i = 0;
    while (i < 1000000 && e() == r.next())
    {
        ++i;
    }

    BOOST_TEST_EQ(i, 1000000);
}

#endif

int main()
{
    const uint64_t a = UINT64_C(0xff4e24dd418f5433);
    const uint64_t max = UINT64_C(0xffffffffffffffff);

    // x = 0x0123456789abcdef, y = 0xfedcba9876543210, c = 1
    const mwc192x start = make_engine<mwc192x>({ UINT64_C(0x0123456789abcdef), UINT64_C(0xfedcba9876543210), 1 });

    // These values were produced with the reference implementation
    check_outputs(start, { UINT64_C(0xfedcba9876543211), UINT64_C(0xac8dcd6abd635774), UINT64_C(0xaa0f361e7b30715d),
                           UINT64_C(0xe104b15c76fca2e4), UINT64_C(0x2c12e1a7db8cbfa1), UINT64_C(0x6105bf55a327d362),
                           UINT64_C(0x62ace12dcd4dc6ba), UINT64_C(0x90e3539226e6de34), UINT64_C(0x112e01165d99bb15),
                           UINT64_C(0x607f209a25bea6ba) });

    // discard() at large distances, with states computed by the reference implementation
    mwc192x e;
    e = start;
    e.discard(12345);
    BOOST_TEST(e == make_engine<mwc192x>({ UINT64_C(0x10ef1947237ccbb6), UINT64_C(0x4de156bc5cf48087), UINT64_C(0xe8c5b59ed41706bd) }));
    e = start;
    e.discard(UINT64_C(0x9e3779b97f4a7c15));
    BOOST_TEST(e == make_engine<mwc192x>({ UINT64_C(0x9a9addfe20bc00c8), UINT64_C(0xbf7da39fa67b2851), UINT64_C(0x1adbc6886c40535a) }));
    e = start;
    e.discard(UINT64_C(0xffffffffffffffff));
    BOOST_TEST(e == make_engine<mwc192x>({ UINT64_C(0x152d4c0143c9cfd7), UINT64_C(0x48eb25ea11450b01), UINT64_C(0x6b0f2e23d7b82181) }));
    e = start;
    e.discard(UINT64_C(1) << 32);
    BOOST_TEST(e == make_engine<mwc192x>({ UINT64_C(0x93e9a4a82fd6439b), UINT64_C(0x1da0e1033c9c2767), UINT64_C(0x3c6f78083948680f) }));
    e = start;
    e.discard(UINT64_C(1) << 48);
    BOOST_TEST(e == make_engine<mwc192x>({ UINT64_C(0x33df4ff1df935014), UINT64_C(0xb479d139117b3c1b), UINT64_C(0x042ebef1828ce592) }));
    // 2^64
    e = start;
    e.discard(UINT64_C(1) << 63);
    e.discard(UINT64_C(1) << 63);
    BOOST_TEST(e == make_engine<mwc192x>({ UINT64_C(0x48eb25ea11450b01), UINT64_C(0xa9e580be41381556), UINT64_C(0x151e958f171fd3b5) }));

    // jump(c, e), with states computed by the reference implementation:
    // jump(1, 96) and jump(1, 144) are its jump() and long_jump()
    e = start;
    e.jump(1, 96);
    BOOST_TEST(e == make_engine<mwc192x>({ UINT64_C(0x5438b996afe69147), UINT64_C(0x4d8061a4a1f2862f), UINT64_C(0xd600e9332b37e5a7) }));
    e = start;
    e.jump(1, 144);
    BOOST_TEST(e == make_engine<mwc192x>({ UINT64_C(0x9ff4a43e2bc6a812), UINT64_C(0xf695800ae27947cf), UINT64_C(0x008f2e60cfea1794) }));
    e = start;
    e.jump(3, 70);
    BOOST_TEST(e == make_engine<mwc192x>({ UINT64_C(0x746e2db78364cb25), UINT64_C(0x5b3243b447ce93d9), UINT64_C(0x6e64eea0ebc89f14) }));
    e = start;
    e.jump(12345, 0);
    BOOST_TEST(e == make_engine<mwc192x>({ UINT64_C(0x10ef1947237ccbb6), UINT64_C(0x4de156bc5cf48087), UINT64_C(0xe8c5b59ed41706bd) }));

    // discard() from the default state, a distance computed with the reference implementation
    e = mwc192x();
    e.discard(1000000);
    BOOST_TEST(e == make_engine<mwc192x>({ UINT64_C(0xcd2e0328326ef24d), UINT64_C(0x01cf511a1e056113), UINT64_C(0xe263fb805f0f7665) }));

    check_discard(start);
    check_discard(mwc192x(42));
    check_discard_composition(start);
    check_jump(start, 96);

    check_streaming(start);
    check_short_seed<mwc192x>();

    // Seeding with an integer: SplitMix digits and carry one
    {
        uint64_t s = 42;
        uint64_t z[2];
        for (auto& w : z)
        {
            w = (s += UINT64_C(0x9E3779B97F4A7C15));
            w = (w ^ (w >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
            w = (w ^ (w >> 27)) * UINT64_C(0x94D049BB133111EB);
            w ^= w >> 31;
        }
        BOOST_TEST(mwc192x(42) == make_engine<mwc192x>({ z[0], z[1], 1 }));
        BOOST_TEST(mwc192x() == mwc192x(0));
    }

    // The zero state and the states not smaller than m get carry one; the
    // other states, including the largest one, are left unchanged.
    BOOST_TEST(make_engine<mwc192x>({ 0, 0, 0 }).state()[2] == 1);
    BOOST_TEST(make_engine<mwc192x>({ max, max, a - 1 }).state()[2] == 1);
    BOOST_TEST(make_engine<mwc192x>({ 5, 0, max }).state()[2] == 1);
    BOOST_TEST(make_engine<mwc192x>({ max, max - 1, a - 1 }).state()[2] == a - 1);
    BOOST_TEST(make_engine<mwc192x>({ 1, 0, 0 }).state()[2] == 0);

#if defined(__SIZEOF_INT128__)
    compare_with_reference(UINT64_C(0x0123456789abcdef), UINT64_C(0xfedcba9876543210), 1);
    compare_with_reference(max, max - 1, a - 1);
    compare_with_reference(max, max, a - 2);
    compare_with_reference(1, 0, 0);
#endif

    return boost::report_errors();
}
