/* test_piecewise_constant_distribution.cpp
 *
 * Copyright Steven Watanabe 2011
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * $Id$
 *
 */

#include <boost/random/piecewise_constant_distribution.hpp>
#include <boost/random/linear_congruential.hpp>
#include <sstream>
#include <vector>
#include "concepts.hpp"

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using boost::random::test::RandomNumberDistribution;
using boost::random::piecewise_constant_distribution;
BOOST_CONCEPT_ASSERT((RandomNumberDistribution< piecewise_constant_distribution<> >));

struct gen {
    double operator()(double arg) {
        if(arg < 100) return 100;
        else if(arg < 103) return 1;
        else if(arg < 107) return 2;
        else if(arg < 111) return 1;
        else if(arg < 114) return 4;
        else return 100;
    }
};

#define CHECK_SEQUENCE(actual, expected)                                    \
    do {                                                                    \
        const std::vector<double>& actual_ = (actual);                      \
        std::vector<double> expected_ = make_vector<double>expected;        \
        BOOST_TEST(actual_ == expected_, boost::test_tools::per_element()); \
    } while(false)

BOOST_AUTO_TEST_CASE(test_constructors) {
    boost::random::piecewise_constant_distribution<> dist;
    CHECK_SEQUENCE(dist.densities(), (1.0));
    CHECK_SEQUENCE(dist.intervals(), (0.0, 1.0));

#ifndef BOOST_NO_CXX11_HDR_INITIALIZER_LIST
    boost::random::piecewise_constant_distribution<> dist_il = {
        { 99, 103, 107, 111, 115 },
        gen()
    };
    CHECK_SEQUENCE(dist_il.intervals(), (99, 103, 107, 111, 115));
    CHECK_SEQUENCE(dist_il.densities(), (.03125, .0625, .03125, .125));

    boost::random::piecewise_constant_distribution<> dist_il2 = {
        { 99 },
        gen()
    };
    CHECK_SEQUENCE(dist_il2.intervals(), (0.0, 1.0));
    CHECK_SEQUENCE(dist_il2.densities(), (1.0));
#endif
    std::vector<double> intervals{ 0, 1, 2, 3, 5 };
    std::vector<double> weights{ 1, 2, 1, 4 };
    std::vector<double> intervals2{ 99 };
    std::vector<double> weights2;

    boost::random::piecewise_constant_distribution<> dist_r(intervals, weights);
    CHECK_SEQUENCE(dist_r.intervals(), (0, 1, 2, 3, 5));
    CHECK_SEQUENCE(dist_r.densities(), (.125, .25, .125, .25));

    boost::random::piecewise_constant_distribution<>
        dist_r2(intervals2, weights2);
    CHECK_SEQUENCE(dist_r2.intervals(), (0.0, 1.0));
    CHECK_SEQUENCE(dist_r2.densities(), (1.0));
    
    boost::random::piecewise_constant_distribution<> dist_it(
        intervals.begin(), intervals.end(), weights.begin());
    CHECK_SEQUENCE(dist_it.intervals(), (0, 1, 2, 3, 5));
    CHECK_SEQUENCE(dist_it.densities(), (.125, .25, .125, .25));
    
    boost::random::piecewise_constant_distribution<> dist_it2(
        intervals2.begin(), intervals2.end(), weights2.begin());
    CHECK_SEQUENCE(dist_it2.intervals(), (0.0, 1.0));
    CHECK_SEQUENCE(dist_it2.densities(), (1.0));
    
    boost::random::piecewise_constant_distribution<> dist_fun(4, 99,115, gen());
    CHECK_SEQUENCE(dist_fun.intervals(), (99, 103, 107, 111, 115));
    CHECK_SEQUENCE(dist_fun.densities(), (.03125, .0625, .03125, .125));
    
    boost::random::piecewise_constant_distribution<>
        dist_fun2(1, 99, 115, gen());
    CHECK_SEQUENCE(dist_fun2.intervals(), (99, 115));
    CHECK_SEQUENCE(dist_fun2.densities(), (0.0625));

    boost::random::piecewise_constant_distribution<> copy(dist);
    BOOST_TEST(dist == copy);
    boost::random::piecewise_constant_distribution<> copy_r(dist_r);
    BOOST_TEST(dist_r == copy_r);

    boost::random::piecewise_constant_distribution<> notpow2(3, 99, 111, gen());
    BOOST_REQUIRE_EQUAL(notpow2.densities().size(), 3u);
    BOOST_TEST(notpow2.densities()[0] == 0.0625, boost::test_tools::tolerance(0.00000000001));
    BOOST_TEST(notpow2.densities()[1] == 0.125, boost::test_tools::tolerance(0.00000000001));
    BOOST_TEST(notpow2.densities()[2] == 0.0625, boost::test_tools::tolerance(0.00000000001));
    boost::random::piecewise_constant_distribution<> copy_notpow2(notpow2);
    BOOST_TEST(notpow2 == copy_notpow2);
}

BOOST_AUTO_TEST_CASE(test_param) {
    std::vector<double> intervals{ 0, 1, 2, 3, 5 };
    std::vector<double> weights{ 1, 2, 1, 4 };
    std::vector<double> intervals2{ 0 };
    std::vector<double> weights2;
    boost::random::piecewise_constant_distribution<> dist(intervals, weights);
    boost::random::piecewise_constant_distribution<>::param_type
        param = dist.param();
    CHECK_SEQUENCE(param.intervals(), (0, 1, 2, 3, 5));
    CHECK_SEQUENCE(param.densities(), (.125, .25, .125, .25));
    boost::random::piecewise_constant_distribution<> copy1(param);
    BOOST_TEST(dist == copy1);
    boost::random::piecewise_constant_distribution<> copy2;
    copy2.param(param);
    BOOST_TEST(dist == copy2);

    boost::random::piecewise_constant_distribution<>::param_type
        param_copy = param;
    BOOST_TEST(param == param_copy);
    BOOST_CHECK(param == param_copy);
    BOOST_CHECK(!(param != param_copy));
    boost::random::piecewise_constant_distribution<>::param_type param_default;
    CHECK_SEQUENCE(param_default.intervals(), (0.0, 1.0));
    CHECK_SEQUENCE(param_default.densities(), (1.0));
    BOOST_CHECK(param != param_default);
    BOOST_CHECK(!(param == param_default));
    
#ifndef BOOST_NO_CXX11_HDR_INITIALIZER_LIST
    boost::random::piecewise_constant_distribution<>::param_type parm_il = {
        { 99, 103, 107, 111, 115 },
        gen()
    };
    CHECK_SEQUENCE(parm_il.intervals(), (99, 103, 107, 111, 115));
    CHECK_SEQUENCE(parm_il.densities(), (.03125, .0625, .03125, .125));

    boost::random::piecewise_constant_distribution<>::param_type parm_il2 = {
        { 99 },
        gen()
    };
    CHECK_SEQUENCE(parm_il2.intervals(), (0.0, 1.0));
    CHECK_SEQUENCE(parm_il2.densities(), (1.0));
#endif

    boost::random::piecewise_constant_distribution<>::param_type
        parm_r(intervals, weights);
    CHECK_SEQUENCE(parm_r.intervals(), (0, 1, 2, 3, 5));
    CHECK_SEQUENCE(parm_r.densities(), (.125, .25, .125, .25));

    boost::random::piecewise_constant_distribution<>::param_type
        parm_r2(intervals2, weights2);
    CHECK_SEQUENCE(parm_r2.intervals(), (0.0, 1.0));
    CHECK_SEQUENCE(parm_r2.densities(), (1.0));
    
    boost::random::piecewise_constant_distribution<>::param_type
        parm_it(intervals.begin(), intervals.end(), weights.begin());
    CHECK_SEQUENCE(parm_it.intervals(), (0, 1, 2, 3, 5));
    CHECK_SEQUENCE(parm_it.densities(), (.125, .25, .125, .25));
    
    boost::random::piecewise_constant_distribution<>::param_type
        parm_it2(intervals2.begin(), intervals2.end(), weights2.begin());
    CHECK_SEQUENCE(parm_it2.intervals(), (0.0, 1.0));
    CHECK_SEQUENCE(parm_it2.densities(), (1.0));
    
    boost::random::piecewise_constant_distribution<>::param_type
        parm_fun(4, 99, 115, gen());
    CHECK_SEQUENCE(parm_fun.intervals(), (99, 103, 107, 111, 115));
    CHECK_SEQUENCE(parm_fun.densities(), (.03125, .0625, .03125, .125));
    
    boost::random::piecewise_constant_distribution<>::param_type
        parm_fun2(1, 99, 115, gen());
    CHECK_SEQUENCE(parm_fun2.intervals(), (99, 115));
    CHECK_SEQUENCE(parm_fun2.densities(), (0.0625));
}

BOOST_AUTO_TEST_CASE(test_min_max) {
    std::vector<double> intervals{ 0, 1, 2, 3, 5 };
    std::vector<double> weights{ 1, 2, 1, 4 };
    boost::random::piecewise_constant_distribution<> dist;
    BOOST_TEST((dist.min)() == 0.0);
    BOOST_TEST((dist.max)() == 1.0);
    boost::random::piecewise_constant_distribution<> dist_r(intervals, weights);
    BOOST_TEST((dist_r.min)() == 0.0);
    BOOST_TEST((dist_r.max)() == 5.0);
}

BOOST_AUTO_TEST_CASE(test_comparison) {
    std::vector<double> intervals{ 0, 1, 2, 3, 5 };
    std::vector<double> weights{ 1, 2, 1, 4 };
    boost::random::piecewise_constant_distribution<> dist;
    boost::random::piecewise_constant_distribution<> dist_copy(dist);
    boost::random::piecewise_constant_distribution<> dist_r(intervals, weights);
    boost::random::piecewise_constant_distribution<> dist_r_copy(dist_r);
    BOOST_CHECK(dist == dist_copy);
    BOOST_CHECK(!(dist != dist_copy));
    BOOST_CHECK(dist_r == dist_r_copy);
    BOOST_CHECK(!(dist_r != dist_r_copy));
    BOOST_CHECK(dist != dist_r);
    BOOST_CHECK(!(dist == dist_r));
}

BOOST_AUTO_TEST_CASE(test_streaming) {
    std::vector<double> intervals{ 0, 1, 2, 3, 5 };
    std::vector<double> weights{ 1, 2, 1, 4 };
    boost::random::piecewise_constant_distribution<> dist(intervals, weights);
    std::stringstream stream;
    stream << dist;
    boost::random::piecewise_constant_distribution<> restored_dist;
    stream >> restored_dist;
    BOOST_TEST(dist == restored_dist);
}

BOOST_AUTO_TEST_CASE(test_generation) {
    std::vector<double> intervals{ 1, 2 };
    std::vector<double> weights{ 1 };
    boost::minstd_rand0 gen;
    boost::random::piecewise_constant_distribution<> dist;
    boost::random::piecewise_constant_distribution<> dist_r(intervals, weights);
    for(int i = 0; i < 10; ++i) {
        double value = dist(gen);
        BOOST_TEST(value >= 0.0);
        BOOST_TEST(value < 1.0);
        double value_r = dist_r(gen);
        BOOST_TEST(value_r >= 1.0);
        BOOST_TEST(value_r < 2.0);
        double value_param = dist_r(gen, dist.param());
        BOOST_TEST(value_param >= 0.0);
        BOOST_TEST(value_param < 1.0);
        double value_r_param = dist(gen, dist_r.param());
        BOOST_TEST(value_r_param >= 1.0);
        BOOST_TEST(value_r_param < 2.0);
    }
}
