/* test_hyperexponential_distribution.ipp
 *
 * Copyright 2014 Marco Guazzone (marco.guazzone@gmail.com)
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 */


#include <boost/math/tools/precision.hpp>
#include <boost/random/hyperexponential_distribution.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/lagged_fibonacci.hpp>
#include <boost/assign/list_of.hpp>
#include <limits>
#include <sstream>
#include <vector>

#include "concepts.hpp"

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>


#define BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(T, actual, expected, tol)\
    do {                                                                       \
        std::vector<T> x = (actual);                                           \
        std::vector<T> y = (expected);                                         \
        BOOST_CHECK_EQUAL( x.size(), y.size() );                               \
        const std::size_t n = x.size();                                        \
        for (std::size_t i = 0; i < n; ++i)                                    \
        {                                                                      \
            BOOST_CHECK_CLOSE( x[i], y[i], tol );                              \
        }                                                                      \
    } while(false)



namespace /*<unnamed>*/ { namespace detail {

template <typename RealT>
RealT make_tolerance()
{
    // Tolerance is 100eps expressed as a percentage (as required by Boost.Build):
    return boost::math::tools::epsilon<RealT>() * 100 * 100;
}

}} // Namespace <unnamed>::detail


BOOST_CONCEPT_ASSERT((boost::random::test::RandomNumberDistribution< boost::random::hyperexponential_distribution<> >));

BOOST_AUTO_TEST_CASE( test_constructors )
{
	const double tol = detail::make_tolerance<double>();

	// Test default ctor
    boost::random::hyperexponential_distribution<> dist;
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, dist.probabilities(), boost::assign::list_of(1.0), tol);
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, dist.rates(), boost::assign::list_of(1.0), tol);

#ifndef BOOST_NO_CXX11_HDR_INITIALIZER_LIST
	// Test ctor from initializer_list
    boost::random::hyperexponential_distribution<> dist_il = {{1, 2, 3, 4 }, {1, 2, 3, 4}};
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, dist_il.probabilities(), list_of(.1)(.2)(.3)(.4), tol);
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, dist_il.rates(), list_of(1.)(2.)(3.)(4.), tol);
#endif

    const std::vector<double> probs = boost::assign::list_of(0.1)(0.2)(0.3)(0.4);
    const std::vector<double> rates = boost::assign::list_of(1.0)(2.0)(3.0)(4.0);

	// Test ctor from range
    boost::random::hyperexponential_distribution<> dist_r(probs, rates);
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, dist_r.probabilities(), probs, tol);
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, dist_r.rates(), rates, tol);

	// Test ctor from iterators
    boost::random::hyperexponential_distribution<> dist_it(probs.begin(), probs.end(), rates.begin(), rates.end());
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, dist_it.probabilities(), probs, tol);
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, dist_it.rates(), rates, tol);

	// Test copy ctor
    boost::random::hyperexponential_distribution<> cp(dist);
	BOOST_CHECK_EQUAL(cp, dist);
    boost::random::hyperexponential_distribution<> cp_r(dist_r);
	BOOST_CHECK_EQUAL(cp_r, dist_r);
}

BOOST_AUTO_TEST_CASE( test_param )
{
	const double tol = detail::make_tolerance<double>();

    const std::vector<double> probs = boost::assign::list_of(0.1)(0.2)(0.3)(0.4);
    const std::vector<double> rates = boost::assign::list_of(1.0)(2.0)(3.0)(4.0);

	// Test param getter
	boost::random::hyperexponential_distribution<> dist(probs, rates);
	boost::random::hyperexponential_distribution<>::param_type param = dist.param();
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, dist.probabilities(), param.probabilities(), tol);
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, dist.rates(), param.rates(), tol);

	// Test ctor from param
	boost::random::hyperexponential_distribution<> cp1(param);
	BOOST_CHECK_EQUAL(cp1, dist);

	// Test param setter
	boost::random::hyperexponential_distribution<> cp2;
	cp2.param(param);
	BOOST_CHECK_EQUAL(cp2, dist);

	// Test param constructors & operators
    boost::random::hyperexponential_distribution<>::param_type param_copy = param;
    BOOST_CHECK_EQUAL(param, param_copy);
    BOOST_CHECK(param == param_copy);
    BOOST_CHECK(!(param != param_copy));
    boost::random::hyperexponential_distribution<>::param_type param_default;
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, param_default.probabilities(), boost::assign::list_of(1.0), tol);
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, param_default.rates(), boost::assign::list_of(1.0), tol);
    BOOST_CHECK(param != param_default);
    BOOST_CHECK(!(param == param_default));
#ifndef BOOST_NO_CXX11_HDR_INITIALIZER_LIST
    boost::random::hyperexponential_distribution<>::param_type param_il = {{1, 2, 3, 4 }, {1, 2, 3, 4}};
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, param_il.probabilities(), list_of(.1)(.2)(.3)(.4), tol);
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, param_il.rates(), list_of(1.)(2.)(3.)(4.), tol);
#endif
    boost::random::hyperexponential_distribution<>::param_type param_r(probs, rates);
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, param_r.probabilities(), probs, tol);
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, param_r.rates(), rates, tol);

    boost::random::hyperexponential_distribution<>::param_type param_it(probs.begin(), probs.end(), rates.begin(), rates.end());
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, param_it.probabilities(), probs, tol);
    BOOST_RANDOM_HYPEREXP_CHECK_CLOSE_COLLECTIONS(double, param_it.rates(), rates, tol);
}

BOOST_AUTO_TEST_CASE( test_min_max )
{
	//const double tol = detail::make_tolerance<double>();

    const std::vector<double> probs = boost::assign::list_of(0.1)(0.2)(0.3)(0.4);
    const std::vector<double> rates = boost::assign::list_of(1.0)(2.0)(3.0)(4.0);

    boost::random::hyperexponential_distribution<> dist;
    BOOST_CHECK_EQUAL((dist.min)(), 0);
    BOOST_CHECK_EQUAL((dist.max)(), (std::numeric_limits<double>::infinity)());

    boost::random::hyperexponential_distribution<> dist_r(probs, rates);
    BOOST_CHECK_EQUAL((dist_r.min)(), 0);
    BOOST_CHECK_EQUAL((dist_r.max)(), (std::numeric_limits<double>::infinity)());
}

BOOST_AUTO_TEST_CASE(test_comparison)
{
	//const double tol = detail::make_tolerance<double>();

    const std::vector<double> probs = boost::assign::list_of(0.1)(0.2)(0.3)(0.4);
    const std::vector<double> rates = boost::assign::list_of(1.0)(2.0)(3.0)(4.0);

    boost::random::hyperexponential_distribution<> dist;
    boost::random::hyperexponential_distribution<> dist_copy(dist);

    boost::random::hyperexponential_distribution<> dist_r(probs, rates);
    boost::random::hyperexponential_distribution<> dist_r_copy(dist_r);

    BOOST_CHECK(dist == dist_copy);
    BOOST_CHECK(!(dist != dist_copy));
    BOOST_CHECK(dist_r == dist_r_copy);
    BOOST_CHECK(!(dist_r != dist_r_copy));
    BOOST_CHECK(dist != dist_r);
    BOOST_CHECK(!(dist == dist_r));
}

BOOST_AUTO_TEST_CASE( test_streaming )
{
	//const double tol = detail::make_tolerance<double>();

    const std::vector<double> probs = boost::assign::list_of(0.1)(0.2)(0.3)(0.4);
    const std::vector<double> rates = boost::assign::list_of(1.0)(2.0)(3.0)(4.0);

    boost::random::hyperexponential_distribution<> dist(probs, rates);
	std::stringstream ss;
	ss << dist;
    boost::random::hyperexponential_distribution<> restored_dist;
	ss >> restored_dist;
	BOOST_CHECK_EQUAL(dist, restored_dist);
}

void use(boost::random::hyperexponential_distribution<>::result_type) {}

BOOST_AUTO_TEST_CASE(test_generation)
{
	//const double tol = detail::make_tolerance<double>();

    const std::vector<double> probs = boost::assign::list_of(0.1)(0.2)(0.3)(0.4);
    const std::vector<double> rates = boost::assign::list_of(1.0)(2.0)(3.0)(4.0);

    boost::minstd_rand0 gen;

    boost::random::hyperexponential_distribution<> dist;
    boost::random::hyperexponential_distribution<> dist_r(probs, rates);
    typedef boost::random::hyperexponential_distribution<>::result_type result_type;
    for(std::size_t i = 0; i < 10; ++i)
	{
        const result_type value = dist(gen);
        use(value);
        BOOST_CHECK_GE(value, static_cast<result_type>(0));
        const result_type value_r = dist_r(gen);
        use(value_r);
        BOOST_CHECK_GE(value_r, static_cast<result_type>(0));
        const result_type value_param = dist_r(gen, dist.param());
        use(value_param);
        BOOST_CHECK_GE(value_param, static_cast<result_type>(0));
        const result_type value_r_param = dist(gen, dist_r.param());
        use(value_r_param);
        BOOST_CHECK_GE(value_r_param, static_cast<result_type>(0));
    }
}

BOOST_AUTO_TEST_CASE( test_generation_float )
{
	//const double tol = detail::make_tolerance<double>();

    const std::vector<double> probs = boost::assign::list_of(0.1)(0.2)(0.3)(0.4);
    const std::vector<double> rates = boost::assign::list_of(1.0)(2.0)(3.0)(4.0);

    boost::lagged_fibonacci607 gen;

    boost::random::hyperexponential_distribution<> dist;
    boost::random::hyperexponential_distribution<> dist_r(probs, rates);
    typedef boost::random::hyperexponential_distribution<>::result_type result_type;
    for(std::size_t i = 0; i < 10; ++i)
	{
        const result_type value = dist(gen);
        use(value);
        BOOST_CHECK_GE(value, static_cast<result_type>(0));
        const result_type value_r = dist_r(gen);
        use(value_r);
        BOOST_CHECK_GE(value_r, static_cast<result_type>(0));
        const result_type value_param = dist_r(gen, dist.param());
        use(value_param);
        BOOST_CHECK_GE(value_param, static_cast<result_type>(0));
        const result_type value_r_param = dist(gen, dist_r.param());
        use(value_r_param);
        BOOST_CHECK_GE(value_r_param, static_cast<result_type>(0));
    }
}