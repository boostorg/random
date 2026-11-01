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

using boost::random::mwc64x32;
using std::uint32_t;
using std::uint64_t;

/* The reference implementation (https://github.com/vigna/MWCPRNG/blob/main/prngs/MWC64X_32.c),
   with the state in a structure. */

#define MWC_A1 0xfff145f8

struct reference
{
    uint32_t x, c;

    uint32_t next(void) {
        const uint32_t result = x ^ c;
        const uint64_t t = MWC_A1 * (uint64_t)x + c;
        x = t;
        c = t >> 32;
        return result;
    }
};

static void compare_with_reference(const uint32_t x, const uint32_t c)
{
    reference r = { x, c };
    mwc64x32 e = make_engine<mwc64x32>({ x, c });
    for (int i = 0; i < 1000000; ++i)
    {
        if (e() != r.next())
        {
            BOOST_ERROR("mwc64x32 differs from the reference implementation");
            return;
        }
    }
}

int main()
{
    const uint32_t a = UINT32_C(0xfff145f8);
    const uint32_t max = UINT32_C(0xffffffff);

    // x = 0x89abcdef, c = 1
    const mwc64x32 start = make_engine<mwc64x32>({ UINT32_C(0x89abcdef), 1 });

    // These values were produced with the reference implementation
    check_outputs(start, { UINT32_C(0x89abcdee), UINT32_C(0x124c08f7), UINT32_C(0x1dd9f62f), UINT32_C(0xe3dcdf5f),
                           UINT32_C(0x6aadea99), UINT32_C(0x3b18a0d9), UINT32_C(0x8a555e1f), UINT32_C(0x46d69f8e),
                           UINT32_C(0x9edf4569), UINT32_C(0x85ccb634) });

    // discard() at large distances, with states computed by the reference implementation
    mwc64x32 e;
    e = start;
    e.discard(12345);
    BOOST_TEST(e == make_engine<mwc64x32>({ UINT32_C(0x8304459c), UINT32_C(0xc3636383) }));
    e = start;
    e.discard(UINT64_C(0x9e3779b97f4a7c15));
    BOOST_TEST(e == make_engine<mwc64x32>({ UINT32_C(0x56b9861b), UINT32_C(0x0a7a33b3) }));
    e = start;
    e.discard(UINT64_C(0xffffffffffffffff));
    BOOST_TEST(e == make_engine<mwc64x32>({ UINT32_C(0x0b5d9549), UINT32_C(0x367c459d) }));
    e = start;
    e.discard(UINT64_C(1) << 32); // jump() of the reference implementation
    BOOST_TEST(e == make_engine<mwc64x32>({ UINT32_C(0xf9132140), UINT32_C(0xd28b3488) }));
    e = start;
    e.discard(UINT64_C(1) << 48); // long_jump() of the reference implementation
    BOOST_TEST(e == make_engine<mwc64x32>({ UINT32_C(0x48cd3df2), UINT32_C(0x5b545e15) }));
    // 2^64
    e = start;
    e.discard(UINT64_C(1) << 63);
    e.discard(UINT64_C(1) << 63);
    BOOST_TEST(e == make_engine<mwc64x32>({ UINT32_C(0xb31a9155), UINT32_C(0x0b5cede8) }));

    // jump(c, e), with states computed by the reference implementation:
    // jump(1, 32) and jump(1, 48) are its jump() and long_jump()
    e = start;
    e.jump(1, 32);
    BOOST_TEST(e == make_engine<mwc64x32>({ UINT32_C(0xf9132140), UINT32_C(0xd28b3488) }));
    e = start;
    e.jump(1, 48);
    BOOST_TEST(e == make_engine<mwc64x32>({ UINT32_C(0x48cd3df2), UINT32_C(0x5b545e15) }));
    e = start;
    e.jump(3, 70);
    BOOST_TEST(e == make_engine<mwc64x32>({ UINT32_C(0xf99803f0), UINT32_C(0x8856dbf7) }));
    e = start;
    e.jump(12345, 0);
    BOOST_TEST(e == make_engine<mwc64x32>({ UINT32_C(0x8304459c), UINT32_C(0xc3636383) }));

    // discard() from the default state, a distance computed with the reference implementation
    e = mwc64x32();
    e.discard(1000000);
    BOOST_TEST(e == make_engine<mwc64x32>({ UINT32_C(0x0afc4416), UINT32_C(0x2a3869bb) }));

    check_discard(start);
    check_discard(mwc64x32(42));
    check_discard_composition(start);
    check_jump(start, 32);

    check_streaming(start);

    // Seeding with an integer: the lower half of a SplitMix output, and carry one
    {
        uint64_t s = 42;
        uint64_t w = (s += UINT64_C(0x9E3779B97F4A7C15));
        w = (w ^ (w >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
        w = (w ^ (w >> 27)) * UINT64_C(0x94D049BB133111EB);
        w ^= w >> 31;
        BOOST_TEST(mwc64x32(42) == make_engine<mwc64x32>({ static_cast<uint32_t>(w), 1 }));
        BOOST_TEST(mwc64x32() == mwc64x32(0));
    }

    // The zero state and the states not smaller than m get carry one; the
    // other states, including the largest one, are left unchanged.
    BOOST_TEST(make_engine<mwc64x32>({ 0, 0 }).state()[1] == 1);
    BOOST_TEST(make_engine<mwc64x32>({ max, a - 1 }).state()[1] == 1);
    BOOST_TEST(make_engine<mwc64x32>({ 5, max }).state()[1] == 1);
    BOOST_TEST(make_engine<mwc64x32>({ max - 1, a - 1 }).state()[1] == a - 1);
    BOOST_TEST(make_engine<mwc64x32>({ 1, 0 }).state()[1] == 0);

    compare_with_reference(UINT32_C(0x89abcdef), 1);
    compare_with_reference(max - 1, a - 1);
    compare_with_reference(max, a - 2);
    compare_with_reference(1, 0);

    return boost::report_errors();
}
