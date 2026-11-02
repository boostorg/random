/*
 * Copyright Sebastiano Vigna 2026.
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * Common checks of the scrambled multiply-with-carry generators (mwcx.hpp).
 */

#ifndef BOOST_RANDOM_TEST_COMP_MWCX_HPP
#define BOOST_RANDOM_TEST_COMP_MWCX_HPP

#include <boost/random/mwcx.hpp>
#include <boost/core/lightweight_test.hpp>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <sstream>
#include <stdexcept>
#include <vector>

// Builds a generator from its state: the digits, oldest first, and then the carry.
template <typename Engine>
Engine make_engine(std::initializer_list<typename Engine::result_type> state)
{
    const std::vector<typename Engine::result_type> v(state);
    Engine e;
    e.seed(v.begin(), v.end());
    return e;
}

// Checks that the first outputs of a generator are the expected ones.
template <typename Engine>
void check_outputs(Engine e, std::initializer_list<typename Engine::result_type> expected)
{
    for (const auto x : expected)
    {
        BOOST_TEST_EQ(e(), x);
    }
}

// Checks that discard(d) is equivalent to d calls to operator()() and to
// jump(d, 0), across the thresholds below which discard() steps the generator.
template <typename Engine>
void check_discard(const Engine& start)
{
    static const std::uint64_t distances[] = { 0, 1, 2, 3, 7, 64, 1000, 4095, 4096, 4097, 9307, 16383, 16384, 16385, 65535, 65536, 65537, 100000 };
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

// Checks that jump(c, e) is equivalent to jump(c * 2^e, 0) when c * 2^e
// fits in 64 bits, and that jump(c1, e) followed by jump(c2, e) is
// equivalent to jump(c1 + c2, e).
template <typename Engine>
void check_jump(const Engine& start, const unsigned int jump_log)
{
    static const std::uint64_t ce[][2] = { { 3, 8 }, { 5, 20 }, { UINT64_C(0x1234567), 30 }, { 1, 63 }, { UINT64_C(0x9e3779b97f4a7c15), 0 } };
    for (const auto& p : ce)
    {
        Engine a = start;
        a.jump(p[0], static_cast<unsigned int>(p[1]));
        Engine b = start;
        b.jump(p[0] << p[1], 0);
        BOOST_TEST(a == b);
    }

    const unsigned int es[] = { 0, 1, 64, jump_log, 2 * jump_log };
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

// Checks that operator<< and operator>> preserve the state.
template <typename Engine>
void check_streaming(const Engine& e)
{
    std::stringstream ss;
    ss << e;
    Engine f;
    ss >> f;
    BOOST_TEST(e == f);
}

// Checks that seeding from a range with fewer values than the state throws.
template <typename Engine>
void check_short_seed()
{
    const std::size_t k = Engine().state().size();
    const std::vector<typename Engine::result_type> v(k - 1, 1);
    for (std::size_t n = 0; n < k; ++n)
    {
        Engine e;
        BOOST_TEST_THROWS(e.seed(v.begin(), v.begin() + n), std::invalid_argument);
        auto first = v.begin();
        BOOST_TEST_THROWS(Engine(first, v.begin() + n), std::invalid_argument);
    }
}

#endif // BOOST_RANDOM_TEST_COMP_MWCX_HPP
