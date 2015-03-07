
/* multiprecision_int_test.cpp
*
* Copyright John Maddock 2015
* Distributed under the Boost Software License, Version 1.0. (See
* accompanying file LICENSE_1_0.txt or copy at
* http://www.boost.org/LICENSE_1_0.txt)
*
* $Id$
*
* Tests all integer related generators and distributions with multiprecision types:
* discard_block, independent_bits_engine, random_number_generator,
* xor_combine_engine, uniform_int_distribution, uniform_smallint.
*
* Not supported, but could be with more work (but probably not worth while):
* shuffle_order_engine, binomial_distribution, discrete_distribution, negative_binomial_distribution,
* poisson_distribution
*/

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/debug_adaptor.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/random.hpp>
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
