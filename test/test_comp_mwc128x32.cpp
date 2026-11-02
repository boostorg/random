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

using boost::random::mwc128x32;
using std::uint32_t;
using std::uint64_t;

/* The reference implementation (https://github.com/vigna/MWCPRNG/blob/main/prngs/MWC128X_32.c),
   with the state in a structure. */

#define MWC_A3 0xffcccc66

struct reference
{
    uint32_t x, y, z, c;

    uint32_t next(void) {
        const uint32_t result = z ^ c;
        const uint64_t t = MWC_A3 * (uint64_t)x + c;
        x = y;
        y = z;
        z = t;
        c = t >> 32;
        return result;
    }
};

static void compare_with_reference(const uint32_t x, const uint32_t y, const uint32_t z, const uint32_t c)
{
    reference r = { x, y, z, c };
    mwc128x32 e = make_engine<mwc128x32>({ x, y, z, c });
    // The index of the first output that differs from the reference implementation
    int i = 0;
    while (i < 1000000 && e() == r.next())
    {
        ++i;
    }

    BOOST_TEST_EQ(i, 1000000);
}

int main()
{
    const uint32_t a = UINT32_C(0xffcccc66);
    const uint32_t max = UINT32_C(0xffffffff);

    // x = 0x89abcdef, y = 0x76543210, z = 0x4b5a6978, c = 1
    const mwc128x32 start = make_engine<mwc128x32>({ UINT32_C(0x89abcdef), UINT32_C(0x76543210), UINT32_C(0x4b5a6978), 1 });

    // These values were produced with the reference implementation
    check_outputs(start, { UINT32_C(0x4b5a6979), UINT32_C(0x6792c5cf), UINT32_C(0xedfc7024), UINT32_C(0x50a57a05),
                           UINT32_C(0x329b3e06), UINT32_C(0x1f1db247), UINT32_C(0x4c7e40d2), UINT32_C(0x3cbaba78),
                           UINT32_C(0x687c6b34), UINT32_C(0xe8c71e6d) });

    // discard() at large distances, with states computed by the reference implementation
    mwc128x32 e;
    e = start;
    e.discard(12345);
    BOOST_TEST(e == make_engine<mwc128x32>({ UINT32_C(0x2602fd2c), UINT32_C(0xd27ae5a4), UINT32_C(0x6dc5ec3d), UINT32_C(0x593ac844) }));
    e = start;
    e.discard(UINT64_C(0x9e3779b97f4a7c15));
    BOOST_TEST(e == make_engine<mwc128x32>({ UINT32_C(0x6385879f), UINT32_C(0xf62de3b4), UINT32_C(0x3028e545), UINT32_C(0xf801572e) }));
    e = start;
    e.discard(UINT64_C(0xffffffffffffffff));
    BOOST_TEST(e == make_engine<mwc128x32>({ UINT32_C(0x4e12e1e3), UINT32_C(0x69ec7c23), UINT32_C(0x85620075), UINT32_C(0x6071af3a) }));
    e = start;
    e.discard(UINT64_C(1) << 32);
    BOOST_TEST(e == make_engine<mwc128x32>({ UINT32_C(0x3988a8aa), UINT32_C(0xe38a592d), UINT32_C(0x18281c8a), UINT32_C(0xfd2a52db) }));
    e = start;
    e.discard(UINT64_C(1) << 48);
    BOOST_TEST(e == make_engine<mwc128x32>({ UINT32_C(0x2b1169fc), UINT32_C(0x28df4959), UINT32_C(0x08847838), UINT32_C(0x3d0297fd) }));
    // 2^64, the distance of jump() in the reference implementation
    e = start;
    e.discard(UINT64_C(1) << 63);
    e.discard(UINT64_C(1) << 63);
    BOOST_TEST(e == make_engine<mwc128x32>({ UINT32_C(0x69ec7c23), UINT32_C(0x85620075), UINT32_C(0xa5dc93ac), UINT32_C(0x4e034463) }));

    // jump(c, e), with states computed by the reference implementation:
    // jump(1, 64) and jump(1, 96) are its jump() and long_jump()
    e = start;
    e.jump(1, 64);
    BOOST_TEST(e == make_engine<mwc128x32>({ UINT32_C(0x69ec7c23), UINT32_C(0x85620075), UINT32_C(0xa5dc93ac), UINT32_C(0x4e034463) }));
    e = start;
    e.jump(1, 96);
    BOOST_TEST(e == make_engine<mwc128x32>({ UINT32_C(0xee6eebbd), UINT32_C(0x140f21ca), UINT32_C(0x45518300), UINT32_C(0x1321a37d) }));
    e = start;
    e.jump(3, 70);
    BOOST_TEST(e == make_engine<mwc128x32>({ UINT32_C(0x8cb9387d), UINT32_C(0x68a3bfad), UINT32_C(0x2d7e249b), UINT32_C(0x7b14417a) }));
    e = start;
    e.jump(12345, 0);
    BOOST_TEST(e == make_engine<mwc128x32>({ UINT32_C(0x2602fd2c), UINT32_C(0xd27ae5a4), UINT32_C(0x6dc5ec3d), UINT32_C(0x593ac844) }));

    // discard() from the default state, a distance computed with the reference implementation
    e = mwc128x32();
    e.discard(1000000);
    BOOST_TEST(e == make_engine<mwc128x32>({ UINT32_C(0x67dd14bf), UINT32_C(0x2079e61c), UINT32_C(0x08abc63e), UINT32_C(0xcbd6c060) }));

    check_discard(start);
    check_discard(mwc128x32(42));
    check_discard_composition(start);
    check_jump(start, 64);

    check_streaming(start);
    check_short_seed<mwc128x32>();

    // Seeding with an integer: the lower and upper halves of two SplitMix
    // outputs (the upper half of the second is not used), and carry one
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
        BOOST_TEST(mwc128x32(42) == make_engine<mwc128x32>({ static_cast<uint32_t>(z[0]), static_cast<uint32_t>(z[0] >> 32), static_cast<uint32_t>(z[1]), 1 }));
        BOOST_TEST(mwc128x32() == mwc128x32(0));
    }

    // The zero state and the states not smaller than m get carry one; the
    // other states, including the largest one, are left unchanged.
    BOOST_TEST(make_engine<mwc128x32>({ 0, 0, 0, 0 }).state()[3] == 1);
    BOOST_TEST(make_engine<mwc128x32>({ max, max, max, a - 1 }).state()[3] == 1);
    BOOST_TEST(make_engine<mwc128x32>({ 5, 0, 0, max }).state()[3] == 1);
    BOOST_TEST(make_engine<mwc128x32>({ max, max, max - 1, a - 1 }).state()[3] == a - 1);
    BOOST_TEST(make_engine<mwc128x32>({ 1, 0, 0, 0 }).state()[3] == 0);

    compare_with_reference(UINT32_C(0x89abcdef), UINT32_C(0x76543210), UINT32_C(0x4b5a6978), 1);
    compare_with_reference(max, max, max - 1, a - 1);
    compare_with_reference(max, max, max, a - 2);
    compare_with_reference(1, 0, 0, 0);

    return boost::report_errors();
}
