/*
 * Copyright Sebastiano Vigna 2026.
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Checks of discard() and jump(c, e) of the xoshiro generators.
 */

#include <boost/random/xoshiro.hpp>
#include <boost/core/lightweight_test.hpp>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <vector>

using namespace boost::random;

// Builds a generator from its state.
template <typename Engine>
Engine make_engine(std::initializer_list<typename Engine::seed_type> state)
{
    const std::vector<typename Engine::seed_type> v(state);
    Engine e;
    e.seed(v.begin(), v.end());
    return e;
}

// Checks that discard(d) and jump(d, 0) are equivalent to d calls to
// operator()(), across the thresholds below which discard() steps the
// generator.
template <typename Engine>
void check_discard(const Engine& start)
{
    static const std::uint64_t distances[] = { 0, 1, 2, 3, 7, 64, 1000, 4095, 4096, 4097, 8191, 8192, 8193, 9307,
                                               16383, 16384, 16385, 65537, 100000 };
    for (const std::uint64_t d : distances)
    {
        Engine a = start;
        for (std::uint64_t i = 0; i < d; ++i)
        {
            a();
        }

        Engine b = start;
        b.discard(d);
        BOOST_TEST(a == b);

        Engine c = start;
        c.jump(d, 0);
        BOOST_TEST(a == c);
    }
}

// Checks that discard(u) followed by discard(v) is equivalent to discard(u + v).
template <typename Engine>
void check_discard_composition(const Engine& start)
{
    static const std::uint64_t pairs[][2] = {
        { 1, UINT64_C(0xfffffffffffffffe) },
        { 100000, UINT64_C(0x123456789abcdef) },
        { UINT64_C(0x7fffffffffffffff), UINT64_C(0x8000000000000000) },
        { UINT64_C(0x9e3779b97f4a7c15), UINT64_C(0x0123456789abcdef) }
    };
    for (const auto& p : pairs)
    {
        Engine a = start;
        a.discard(p[0]);
        a.discard(p[1]);
        Engine b = start;
        b.discard(p[0] + p[1]);
        BOOST_TEST(a == b);
    }
}

// Checks jump(c, e) for a generator with 'bits' bits of state, whose jump()
// and long_jump() advance it by 2^jump_log and 2^long_jump_log steps.
template <typename Engine>
void check_jump(const Engine& start, const unsigned int bits, const unsigned int jump_log, const unsigned int long_jump_log)
{
    // jump(1, jump_log) and jump(1, long_jump_log) are jump() and long_jump()
    {
        Engine a = start;
        a.jump(1, jump_log);
        Engine b = start;
        b.jump();
        BOOST_TEST(a == b);

        a = start;
        a.jump(1, long_jump_log);
        b = start;
        b.long_jump();
        BOOST_TEST(a == b);
    }

    // jump(0, e) does nothing
    {
        Engine a = start;
        a.jump(0, 0);
        BOOST_TEST(a == start);
        a.jump(0, jump_log);
        BOOST_TEST(a == start);
    }

    // As the period is 2^bits - 1, a jump of 2^bits steps is a single step
    {
        Engine a = start;
        a.jump(1, bits);
        Engine b = start;
        b();
        BOOST_TEST(a == b);
    }

    // jump(c, e) is equivalent to jump(c * 2^e, 0) when c * 2^e fits in 64 bits
    static const std::uint64_t ce[][2] = { { 3, 8 }, { 5, 20 }, { UINT64_C(0x1234567), 30 }, { 1, 63 }, { UINT64_C(0x9e3779b97f4a7c15), 0 } };
    for (const auto& p : ce)
    {
        Engine a = start;
        a.jump(p[0], static_cast<unsigned int>(p[1]));
        Engine b = start;
        b.jump(p[0] << p[1], 0);
        BOOST_TEST(a == b);
    }

    // jump(c1, e) followed by jump(c2, e) is equivalent to jump(c1 + c2, e)
    const unsigned int es[] = { 0, 1, 64, jump_log, long_jump_log };
    for (const unsigned int e : es)
    {
        Engine a = start;
        a.jump(3, e);
        a.jump(UINT64_C(0x123456789abcdef), e);
        Engine b = start;
        b.jump(3 + UINT64_C(0x123456789abcdef), e);
        BOOST_TEST(a == b);
    }
}

template <typename Engine>
void check_all(const Engine& start, const unsigned int bits, const unsigned int jump_log, const unsigned int long_jump_log)
{
    check_discard(start);
    check_discard(Engine(42));
    check_discard_composition(start);
    check_jump(start, bits, jump_log, long_jump_log);
}

// The states below were computed with jump_ce() of the reference
// implementations at https://prng.di.unimi.it. They depend only on the
// linear engine, so they are the same for all generators with the same state.

template <typename Engine>
void check_reference_128()
{
    const Engine start = make_engine<Engine>({ UINT32_C(0x89abcdef), UINT32_C(0x92a217f3), UINT32_C(0x9b9861f7), UINT32_C(0xa48eabfb) });
    Engine e = start;
    e.discard(12345);
    BOOST_TEST(e == make_engine<Engine>({ UINT32_C(0xfde876d9), UINT32_C(0x67184059), UINT32_C(0xa45d92c9), UINT32_C(0xb9a7a002) }));
    e = start;
    e.discard(UINT64_C(0x9e3779b97f4a7c15));
    BOOST_TEST(e == make_engine<Engine>({ UINT32_C(0x791642c5), UINT32_C(0x575b3521), UINT32_C(0x1391e379), UINT32_C(0x9f89430e) }));
    e = start;
    e.discard(UINT64_C(0xffffffffffffffff));
    BOOST_TEST(e == make_engine<Engine>({ UINT32_C(0x75b734e2), UINT32_C(0x79451d39), UINT32_C(0x7e7614d4), UINT32_C(0x0a70b3f2) }));
    e = start;
    e.jump(3, 70);
    BOOST_TEST(e == make_engine<Engine>({ UINT32_C(0x6a17ac7b), UINT32_C(0xae4fb410), UINT32_C(0x86f3bf52), UINT32_C(0xde12fa78) }));
    e = start;
    e.jump(UINT64_C(0x123456789abcdef), 71);
    BOOST_TEST(e == make_engine<Engine>({ UINT32_C(0xd9ef01fc), UINT32_C(0xbcf9da3f), UINT32_C(0xd7172609), UINT32_C(0x64c3a091) }));

    check_all(start, 128, 64, 96);
}

template <typename Engine>
void check_reference_256()
{
    const Engine start = make_engine<Engine>({ UINT64_C(0x0123456789abcdef), UINT64_C(0xa07e048892a217f3),
                                               UINT64_C(0x3fd8c3a99b9861f7), UINT64_C(0xdf3382caa48eabfb) });
    Engine e = start;
    e.discard(12345);
    BOOST_TEST(e == make_engine<Engine>({ UINT64_C(0x69cf32f8d724cd8b), UINT64_C(0x7c331e0a4fec602e),
                                          UINT64_C(0xd7d968ad7dcb2779), UINT64_C(0xf1e6137266432579) }));
    e = start;
    e.discard(UINT64_C(0x9e3779b97f4a7c15));
    BOOST_TEST(e == make_engine<Engine>({ UINT64_C(0x70281a74ac355027), UINT64_C(0xa42ca095899e9167),
                                          UINT64_C(0x9d1835a0865afdd1), UINT64_C(0xdf4db42b007e379d) }));
    e = start;
    e.discard(UINT64_C(0xffffffffffffffff));
    BOOST_TEST(e == make_engine<Engine>({ UINT64_C(0x91e3122b2590ba02), UINT64_C(0xa608df3af94e5571),
                                          UINT64_C(0x83d5db190ae2b3c3), UINT64_C(0xc2a8e84c4217a2b6) }));
    e = start;
    e.jump(3, 70);
    BOOST_TEST(e == make_engine<Engine>({ UINT64_C(0xdd829b8601731221), UINT64_C(0x8ad107e028c5a37b),
                                          UINT64_C(0xbfc8df29986dd8b4), UINT64_C(0xff9b643b5cb48331) }));
    e = start;
    e.jump(UINT64_C(0x123456789abcdef), 135);
    BOOST_TEST(e == make_engine<Engine>({ UINT64_C(0x13da6f31fe6072de), UINT64_C(0x23d115d50961ce4c),
                                          UINT64_C(0x990383bc8f71e2f3), UINT64_C(0x29e09f9e78b1d2b4) }));

    check_all(start, 256, 128, 192);
}

template <typename Engine>
void check_reference_512()
{
    const Engine start = make_engine<Engine>({ UINT64_C(0x0123456789abcdef), UINT64_C(0xa07e048892a217f3),
                                               UINT64_C(0x3fd8c3a99b9861f7), UINT64_C(0xdf3382caa48eabfb),
                                               UINT64_C(0x7e8e41ebad84f5ff), UINT64_C(0x1de9010cb67b4003),
                                               UINT64_C(0xbd43c02dbf718a07), UINT64_C(0x5c9e7f4ec867d40b) });
    Engine e = start;
    e.discard(12345);
    BOOST_TEST(e == make_engine<Engine>({ UINT64_C(0x670dfab1a4dbd932), UINT64_C(0xde097f4981d3f4d1),
                                          UINT64_C(0xec2e66f685013683), UINT64_C(0xca4df29dc478e3d5),
                                          UINT64_C(0x7d0ffb16c7d5db22), UINT64_C(0x9cb418f689f884dc),
                                          UINT64_C(0x3269b7eed6746a30), UINT64_C(0x6dd164f43c3c3299) }));
    e = start;
    e.discard(UINT64_C(0x9e3779b97f4a7c15));
    BOOST_TEST(e == make_engine<Engine>({ UINT64_C(0x63d256ba236fe699), UINT64_C(0x9eb057bedbf8a9e8),
                                          UINT64_C(0xd55b8f541d8df6bb), UINT64_C(0xcb3cda5604a5c200),
                                          UINT64_C(0x6484c0edfd9c1574), UINT64_C(0x0fb55f7648bc7ce1),
                                          UINT64_C(0x44e19ae84b723000), UINT64_C(0xdb56ea6746568ebb) }));
    e = start;
    e.discard(UINT64_C(0xffffffffffffffff));
    BOOST_TEST(e == make_engine<Engine>({ UINT64_C(0x03d11e951e72419a), UINT64_C(0x775e3fd2797ae991),
                                          UINT64_C(0x6e525c9f87143faf), UINT64_C(0x40f37b28d4a59ecb),
                                          UINT64_C(0xcc5ffade71259ecb), UINT64_C(0x9f8fc85247f83135),
                                          UINT64_C(0xbe8a071b77f1aa3f), UINT64_C(0x4c23cb4260054391) }));
    e = start;
    e.jump(3, 70);
    BOOST_TEST(e == make_engine<Engine>({ UINT64_C(0x15710a2cbcebaddd), UINT64_C(0xca329d2b933fa0e3),
                                          UINT64_C(0x86192c2a1622632f), UINT64_C(0xd7dab9c5a634aff7),
                                          UINT64_C(0x5bc1fd588730cf0f), UINT64_C(0x2da6036b5f331de7),
                                          UINT64_C(0x3630f4bd57b7278c), UINT64_C(0x85e80425a2c59fd8) }));
    e = start;
    e.jump(UINT64_C(0x123456789abcdef), 263);
    BOOST_TEST(e == make_engine<Engine>({ UINT64_C(0x1716bf48194e0779), UINT64_C(0xeeee48e6c78df93d),
                                          UINT64_C(0x29f9411264d13131), UINT64_C(0x54ccce075543e94f),
                                          UINT64_C(0x960e7688bcef2b39), UINT64_C(0x72ff68e6a85adbe6),
                                          UINT64_C(0xd2406b47887bc7ab), UINT64_C(0xf71e945d33fd21f2) }));

    check_all(start, 512, 256, 384);
}

int main()
{
    check_reference_128<xoshiro128pp>();
    check_reference_128<xoshiro128mm>();
    check_reference_128<xoshiro128f>();

    check_reference_256<xoshiro256pp>();
    check_reference_256<xoshiro256mm>();
    check_reference_256<xoshiro256d>();

    check_reference_512<xoshiro512pp>();
    check_reference_512<xoshiro512mm>();
    check_reference_512<xoshiro512d>();

    return boost::report_errors();
}
