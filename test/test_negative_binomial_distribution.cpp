/* test_negative_binomial_distribution.cpp
 *
 * Copyright Steven Watanabe 2010
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * $Id$
 *
 */

#include <boost/random/negative_binomial_distribution.hpp>
#include <boost/random/linear_congruential.hpp>
#include <iomanip>
#include <limits>
#include <sstream>

#define BOOST_RANDOM_DISTRIBUTION boost::random::negative_binomial_distribution<>
#define BOOST_RANDOM_ARG1 k
#define BOOST_RANDOM_ARG2 p
#define BOOST_RANDOM_ARG1_DEFAULT 1
#define BOOST_RANDOM_ARG2_DEFAULT 0.5
#define BOOST_RANDOM_ARG1_VALUE 10
#define BOOST_RANDOM_ARG2_VALUE 0.25

#define BOOST_RANDOM_DIST0_MIN 0
#define BOOST_RANDOM_DIST0_MAX (std::numeric_limits<int>::max)()
#define BOOST_RANDOM_DIST1_MIN 0
#define BOOST_RANDOM_DIST1_MAX (std::numeric_limits<int>::max)()
#define BOOST_RANDOM_DIST2_MIN 0
#define BOOST_RANDOM_DIST2_MAX (std::numeric_limits<int>::max)()

#define BOOST_RANDOM_TEST1_PARAMS
#define BOOST_RANDOM_TEST1_MIN 0
#define BOOST_RANDOM_TEST1_MAX 10

#define BOOST_RANDOM_TEST2_PARAMS (100, 0.5)
#define BOOST_RANDOM_TEST2_MIN 50

#include "test_distribution.ipp"

namespace {

// Forwards to a real engine so non-degenerate draws terminate, and counts every call
struct counting_engine
{
    typedef boost::random::minstd_rand::result_type result_type;

    counting_engine() : calls {0} {}
    static result_type min() { return (boost::random::minstd_rand::min)(); }
    static result_type max() { return (boost::random::minstd_rand::max)(); }
    result_type operator()() { ++calls; return base(); }

    boost::random::minstd_rand base;
    unsigned calls;
};

template<class IntType, class RealType>
void check_degenerate(IntType k, RealType p)
{
    typedef boost::random::negative_binomial_distribution<IntType, RealType> distribution;
    typedef typename distribution::param_type param_type;
    counting_engine engine;
    const param_type param(k, p);
    distribution direct(k, p);
    distribution from_param(param);
    distribution assigned;
    assigned.param(param);
    distribution explicit_param(5, RealType(0.5));
    distribution streamed;
    std::stringstream stream;
    // At default precision min() prints as a subnormal, which libc++ fails to read back
    stream << std::setprecision(std::numeric_limits<RealType>::max_digits10) << direct;
    stream >> streamed;
    BOOST_REQUIRE(!stream.fail());

    for(unsigned i = 0; i < 4; ++i) {
        BOOST_TEST(direct(engine) == IntType(0));
        BOOST_TEST(from_param(engine) == IntType(0));
        BOOST_TEST(assigned(engine) == IntType(0));
        BOOST_TEST(explicit_param(engine, param) == IntType(0));
        BOOST_TEST(streamed(engine) == IntType(0));
    }
    BOOST_TEST(engine.calls == 0u);
    BOOST_TEST(explicit_param.k() == IntType(5));
    BOOST_TEST(explicit_param.p() == RealType(0.5));
}

template<class IntType, class RealType>
void check_zero_target()
{
    const RealType probabilities[] = {
        RealType(0), (std::numeric_limits<RealType>::min)(),
        RealType(0.25), RealType(0.5), RealType(0.75), RealType(1)
    };
    for(unsigned i = 0; i < sizeof(probabilities) / sizeof(probabilities[0]); ++i)
        check_degenerate(IntType(0), probabilities[i]);
}

template<class IntType, class RealType>
void check_unit_probability()
{
    const IntType targets[] = {
        IntType(0), IntType(1), IntType(10), (std::numeric_limits<IntType>::max)()
    };
    for(unsigned i = 0; i < sizeof(targets) / sizeof(targets[0]); ++i)
        check_degenerate(targets[i], RealType(1));
}

}

BOOST_AUTO_TEST_CASE(test_zero_target)
{
    check_zero_target<int, float>();
    check_zero_target<int, double>();
    check_zero_target<int, long double>();
    check_zero_target<boost::int64_t, double>();
    check_zero_target<unsigned, float>();
    check_zero_target<boost::uint64_t, long double>();
}

BOOST_AUTO_TEST_CASE(test_unit_probability)
{
    check_unit_probability<int, float>();
    check_unit_probability<int, double>();
    check_unit_probability<int, long double>();
    check_unit_probability<boost::int64_t, double>();
    check_unit_probability<unsigned, float>();
    check_unit_probability<boost::uint64_t, long double>();
}

// Positive control: a non-degenerate draw must consume the engine,
// so the engine.calls == 0 checks above would catch a sampling call
BOOST_AUTO_TEST_CASE(test_non_degenerate_uses_engine)
{
    counting_engine engine;
    boost::random::negative_binomial_distribution<> dist {1, 0.5};
    for (unsigned i {0}; i < 4; ++i)
    {
        const int value {dist(engine)};
        BOOST_TEST(value >= 0);
    }
    BOOST_TEST(engine.calls > 0u);
}
