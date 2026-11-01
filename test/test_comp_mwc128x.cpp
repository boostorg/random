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

using boost::random::mwc128x;
using std::uint64_t;

#if defined(__SIZEOF_INT128__)

/* The reference implementation (https://github.com/vigna/MWCPRNG/blob/main/prngs/MWC128X.c),
   with the state in a structure. */

#define MWC_A1 0xff6da236274e252e

struct reference
{
    uint64_t x, c;

    uint64_t next(void) {
        const uint64_t result = x ^ c;
        const __uint128_t t = MWC_A1 * (__uint128_t)x + c;
        x = t;
        c = t >> 64;
        return result;
    }
};

static void compare_with_reference(const uint64_t x, const uint64_t c)
{
    reference r = { x, c };
    mwc128x e = make_engine<mwc128x>({ x, c });
    for (int i = 0; i < 1000000; ++i)
    {
        if (e() != r.next())
        {
            BOOST_ERROR("mwc128x differs from the reference implementation");
            return;
        }
    }
}

#endif

int main()
{
    const uint64_t a = UINT64_C(0xff6da236274e252e);
    const uint64_t max = UINT64_C(0xffffffffffffffff);

    // x = 0x0123456789abcdef, c = 1
    const mwc128x start = make_engine<mwc128x>({ UINT64_C(0x0123456789abcdef), 1 });

    // These values were produced with the reference implementation
    check_outputs(start, { UINT64_C(0x0123456789abcdee), UINT64_C(0xba17a27d87b42a4a), UINT64_C(0xa622a4b120a9f538),
                           UINT64_C(0x1bb7f645c6670559), UINT64_C(0x18529bad6097d72b), UINT64_C(0xf501042c00493549),
                           UINT64_C(0x37e0328b3ce80699), UINT64_C(0x958cd94f7eb2e02f), UINT64_C(0x68e32730e418e30a),
                           UINT64_C(0x4519e7cc8909e77d) });

    // discard() at large distances, with states computed by the reference implementation
    mwc128x e;
    e = start;
    e.discard(12345);
    BOOST_TEST(e == make_engine<mwc128x>({ UINT64_C(0xe7ef89c7c08b0a84), UINT64_C(0x63f3b10034fa793a) }));
    e = start;
    e.discard(UINT64_C(0x9e3779b97f4a7c15));
    BOOST_TEST(e == make_engine<mwc128x>({ UINT64_C(0x7f0a05c03102a9ac), UINT64_C(0xe4cf2f5a527eb8a2) }));
    e = start;
    e.discard(UINT64_C(0xffffffffffffffff));
    BOOST_TEST(e == make_engine<mwc128x>({ UINT64_C(0xe4f3ec1d416d5dad), UINT64_C(0x57bae73c39fc0c2f) }));
    e = start;
    e.discard(UINT64_C(1) << 32);
    BOOST_TEST(e == make_engine<mwc128x>({ UINT64_C(0xbdfa9e1fd955e762), UINT64_C(0x1347546687e55513) }));
    e = start;
    e.discard(UINT64_C(1) << 48);
    BOOST_TEST(e == make_engine<mwc128x>({ UINT64_C(0x3b18b6c941ea29e2), UINT64_C(0x8eacc536be1701a0) }));
    // 2^64, the distance of jump() in the reference implementation
    e = start;
    e.discard(UINT64_C(1) << 63);
    e.discard(UINT64_C(1) << 63);
    BOOST_TEST(e == make_engine<mwc128x>({ UINT64_C(0x5a936408afe2e245), UINT64_C(0xe471051f76a629fe) }));

    // jump(c, e), with states computed by the reference implementation:
    // jump(1, 64) and jump(1, 96) are its jump() and long_jump()
    e = start;
    e.jump(1, 64);
    BOOST_TEST(e == make_engine<mwc128x>({ UINT64_C(0x5a936408afe2e245), UINT64_C(0xe471051f76a629fe) }));
    e = start;
    e.jump(1, 96);
    BOOST_TEST(e == make_engine<mwc128x>({ UINT64_C(0x7e2cba541c5f4c51), UINT64_C(0x30a344ed8d373892) }));
    e = start;
    e.jump(3, 70);
    BOOST_TEST(e == make_engine<mwc128x>({ UINT64_C(0x34d3926d1a7906f8), UINT64_C(0x131cbd0e516ab608) }));
    e = start;
    e.jump(12345, 0);
    BOOST_TEST(e == make_engine<mwc128x>({ UINT64_C(0xe7ef89c7c08b0a84), UINT64_C(0x63f3b10034fa793a) }));

    // discard() from the default state, a distance computed with the reference implementation
    e = mwc128x();
    e.discard(1000000);
    BOOST_TEST(e == make_engine<mwc128x>({ UINT64_C(0x45e7e0993ad51097), UINT64_C(0xaad79e0ed8884d55) }));

    check_discard(start);
    check_discard(mwc128x(42));
    check_discard_composition(start);
    check_jump(start, 64);

    check_streaming(start);

    // Seeding with an integer: SplitMix digit and carry one
    {
        uint64_t s = 42;
        uint64_t w = (s += UINT64_C(0x9E3779B97F4A7C15));
        w = (w ^ (w >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
        w = (w ^ (w >> 27)) * UINT64_C(0x94D049BB133111EB);
        w ^= w >> 31;
        BOOST_TEST(mwc128x(42) == make_engine<mwc128x>({ w, 1 }));
        BOOST_TEST(mwc128x() == mwc128x(0));
    }

    // The zero state and the states not smaller than m get carry one; the
    // other states, including the largest one, are left unchanged.
    BOOST_TEST(make_engine<mwc128x>({ 0, 0 }).state()[1] == 1);
    BOOST_TEST(make_engine<mwc128x>({ max, a - 1 }).state()[1] == 1);
    BOOST_TEST(make_engine<mwc128x>({ 5, max }).state()[1] == 1);
    BOOST_TEST(make_engine<mwc128x>({ max - 1, a - 1 }).state()[1] == a - 1);
    BOOST_TEST(make_engine<mwc128x>({ 1, 0 }).state()[1] == 0);

#if defined(__SIZEOF_INT128__)
    compare_with_reference(UINT64_C(0x0123456789abcdef), 1);
    compare_with_reference(max - 1, a - 1);
    compare_with_reference(max, a - 2);
    compare_with_reference(1, 0);
#endif

    return boost::report_errors();
}
