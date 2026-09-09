
/* multiprecision_float_test.cpp
*
* Copyright John Maddock 2015
* Distributed under the Boost Software License, Version 1.0. (See
* accompanying file LICENSE_1_0.txt or copy at
* http://www.boost.org/LICENSE_1_0.txt)
*
* $Id$
*
* Tests all floating point related generators and distributions with multiprecision types.
*/

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <boost/core/ignore_unused.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/debug_adaptor.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/random.hpp>
#include <boost/mpl/list.hpp>
#include <sstream>


typedef boost::multiprecision::number<boost::multiprecision::cpp_bin_float_100::backend_type, boost::multiprecision::et_on > big_float;
typedef boost::random::subtract_with_carry_01_engine<big_float, 48, 10, 24 > ranlux_big_base_01;
typedef boost::random::independent_bits_engine<boost::random::mt19937, 1024, boost::multiprecision::uint1024_t> large_int_generator;

typedef boost::mpl::list <
   boost::random::lagged_fibonacci_01_engine<big_float, 48, 44497, 21034 >,
   boost::random::discard_block_engine< ranlux_big_base_01, 389, 24 >
> engines;

BOOST_AUTO_TEST_CASE_TEMPLATE(generator_test, engine_type, engines)
{
   typedef typename engine_type::result_type test_type;

   boost::scoped_ptr<engine_type> gen(new engine_type());
   unsigned seeds[] = { 1, 2, 3, 4 };
   unsigned *p1 = seeds, *p2 = seeds + 4;
   BOOST_CHECK_THROW(gen->seed(p1, p2), std::invalid_argument);
   gen->seed();
   gen->seed(2);
   test_type a = gen->min();
   test_type b = gen->max();
   BOOST_CHECK(a < b);
   for(unsigned i = 0; i < 200; ++i)
   {
      test_type r = (*gen)();
      BOOST_CHECK((boost::math::isfinite)(r));
      BOOST_CHECK(a <= r);
      BOOST_CHECK(b >= r);
   }
   gen->discard(20);

   std::stringstream ss;
   ss << std::setprecision(std::numeric_limits<test_type>::digits10 + 3) << *gen;
   boost::scoped_ptr<engine_type> gen2(new engine_type());
   ss >> *gen2;
   BOOST_CHECK(*gen == *gen2);
   (*gen2)();
   BOOST_CHECK(*gen != *gen2);
}

typedef boost::mpl::list <
   boost::random::bernoulli_distribution<big_float>,
   boost::random::beta_distribution<big_float>,
   boost::random::cauchy_distribution<big_float>,
   boost::random::chi_squared_distribution<big_float>,
   boost::random::exponential_distribution<big_float>,
   boost::random::extreme_value_distribution<big_float>,
   boost::random::fisher_f_distribution<big_float>,
   boost::random::gamma_distribution<big_float>,
   boost::random::laplace_distribution<big_float>,
   boost::random::lognormal_distribution<big_float>,
   boost::random::normal_distribution<big_float>,
#ifndef BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
   boost::random::piecewise_constant_distribution<big_float, big_float>,
   boost::random::piecewise_linear_distribution<big_float>,
#endif
   boost::random::student_t_distribution<big_float>,
   boost::random::triangle_distribution<big_float>,
   //boost::random::uniform_01<big_float>,  // doesn't respect the concept!  But gets used internally anyway.
   boost::random::uniform_real_distribution<big_float>,
   boost::random::uniform_on_sphere<big_float>,
   boost::random::weibull_distribution<big_float>
> distributions;

template<class T>
void check_result(const T& result, const T& min_val, const T& max_val)
{

   BOOST_CHECK(boost::math::isfinite(result));
   BOOST_TEST(result >= min_val);
   BOOST_TEST(result <= max_val);
}

template<class T>
void check_result(const std::vector<T>& result, const std::vector<T>& min_val, const std::vector<T>& max_val)
{
   for(size_t i = 0; i < result.size(); i++)
   {
      BOOST_CHECK(boost::math::isfinite(result[i]));
   }
   BOOST_TEST(result >= min_val);
   BOOST_TEST(result <= max_val);
}

template<class TDist, class TGen>
void do_check_distribution(TDist& d, TGen& gen)
{
   typedef typename TDist::result_type result_type;
   const result_type min_val = (d.min)();
   const result_type max_val = (d.max)();
   for(unsigned i = 0; i < 200; ++i)
   {
      result_type r = d(gen);
      check_result(r, min_val, max_val);
   }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(distributions_test, dist_type, distributions)
{
   typedef typename dist_type::result_type result_type;
   dist_type d;
   typename dist_type::param_type p = d.param();
   boost::ignore_unused(p);
   d.reset();
   
   std::stringstream ss;
   ss << std::setprecision(std::numeric_limits<result_type>::digits10 + 3) << d;
   dist_type d2;
   ss >> d2;
   BOOST_CHECK(d == d2);

   boost::random::mt19937 int_gen;
   do_check_distribution(d, int_gen);

   large_int_generator big_int_gen;
   do_check_distribution(d, big_int_gen);

   boost::random::discard_block_engine< ranlux_big_base_01, 389, 24 > big_float_gen;
   do_check_distribution(d, big_float_gen);

   boost::random::ranlux64_4_01 float_gen;
   do_check_distribution(d, float_gen);
}



BOOST_AUTO_TEST_CASE(canonical_test)
{
   typedef big_float result_type;

   boost::random::mt19937 int_gen;

   for(unsigned i = 0; i < 200; ++i)
   {
      result_type r = boost::random::generate_canonical<big_float, std::numeric_limits<big_float>::digits>(int_gen);
      BOOST_CHECK((boost::math::isfinite)(r));
      BOOST_CHECK(r >= 0);
      BOOST_CHECK(r <= 1);
   }

   large_int_generator big_int_gen;

   for(unsigned i = 0; i < 200; ++i)
   {
      result_type r = boost::random::generate_canonical<big_float, std::numeric_limits<big_float>::digits>(big_int_gen);
      BOOST_CHECK((boost::math::isfinite)(r));
      BOOST_CHECK(r >= 0);
      BOOST_CHECK(r <= 1);
   }


   boost::random::discard_block_engine< ranlux_big_base_01, 389, 24 > big_float_gen;

   for(unsigned i = 0; i < 200; ++i)
   {
      result_type r = boost::random::generate_canonical<big_float, std::numeric_limits<big_float>::digits>(big_float_gen);
      BOOST_CHECK((boost::math::isfinite)(r));
      BOOST_CHECK(r >= 0);
      BOOST_CHECK(r <= 1);
   }

   boost::random::ranlux64_4_01 float_gen;

   for(unsigned i = 0; i < 200; ++i)
   {
      result_type r = boost::random::generate_canonical<big_float, std::numeric_limits<big_float>::digits>(float_gen);
      BOOST_CHECK((boost::math::isfinite)(r));
      BOOST_CHECK(r >= 0);
      BOOST_CHECK(r <= 1);
   }

}

