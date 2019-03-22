/* test_constexpr.cpp
 *
 * Copyright Ryan Timmons 2019
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * $Id$
 *
 */

#include <boost/random.hpp>
#include <boost/random/random_device.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#if defined BOOST_NO_CXX11_CONSTEXPR
#define BOOST_CONSTEXPR_ASSERT(C) BOOST_TEST(C)
#else
#include <boost/static_assert.hpp>
#define BOOST_CONSTEXPR_ASSERT(C) BOOST_STATIC_ASSERT(C)
#endif

#define TEST_MIN_MAX(C) do { \
    BOOST_CONSTEXPR_ASSERT(C::min() <  C::max()); \
    BOOST_CONSTEXPR_ASSERT(C::max() >  C::min()); \
    BOOST_CONSTEXPR_ASSERT(C::min() == C::min()); \
    BOOST_CONSTEXPR_ASSERT(C::max() == C::max()); \
    } while(0)

BOOST_AUTO_TEST_CASE(test_constexpr)
{
    // additive_combine
    TEST_MIN_MAX(boost::random::ecuyer1988);

    // discard_block and subtract_with_carry
    TEST_MIN_MAX(boost::random::ranlux3);

    // independent_bits
    typedef boost::random::independent_bits_engine<boost::random::mt19937, 1024, boost::multiprecision::uint1024_t> indep_bits;
    TEST_MIN_MAX(indep_bits);
    
    // inversive_congruential
    TEST_MIN_MAX(boost::random::hellekalek1995);

    // lagged_fibonacci
    TEST_MIN_MAX(boost::random::lagged_fibonacci607);

    // linear_congruential
    TEST_MIN_MAX(boost::random::minstd_rand0);

    // linear_feedback_shift and xor_combine
    TEST_MIN_MAX(boost::random::taus88);

    // mersenne_twister
    TEST_MIN_MAX(boost::random::mt19937_64);

    // random_device
    TEST_MIN_MAX(boost::random::random_device);

    // shuffle_order
    TEST_MIN_MAX(boost::random::kreutzer1986);

}

