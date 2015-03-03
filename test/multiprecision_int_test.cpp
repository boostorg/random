
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
* binomial_distribution, discard_block, discrete_distribution, independent_bits_engine,
* negative_binomial_distribution, poisson_distribution, random_number_generator,
* xor_combine_engine, uniform_int_distribution, uniform_smallint.
*
* Not supported, but could be with more work (but probably not worth while:
* shuffle_order_engine
*/

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random/independent_bits.hpp>
#include <boost/random/discard_block.hpp>
#include <boost/random/xor_combine.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/random_number_generator.hpp>
#include <sstream>

typedef boost::mpl::list <
   boost::random::independent_bits_engine<boost::random::mt19937, std::numeric_limits<boost::multiprecision::uint1024_t>::digits, boost::multiprecision::uint1024_t >,
   boost::random::discard_block_engine<boost::random::independent_bits_engine<boost::random::mt19937, std::numeric_limits<boost::multiprecision::uint1024_t>::digits, boost::multiprecision::uint1024_t >, 20, 10>
> engines;

BOOST_AUTO_TEST_CASE_TEMPLATE(generator_test, engine_type, engines)
{
   typedef typename engine_type::result_type test_type;

   engine_type gen;
   gen.seed();
   test_type a = gen.min();
   test_type b = gen.max();
   BOOST_CHECK(a < b);
   a = gen();
   gen.generate(&b, &b + 1);
   gen.discard(20);

   typename engine_type::base_type base(gen.base());

   std::stringstream ss;
   ss << gen;
   engine_type gen2;
   ss >> gen2;
   BOOST_CHECK(gen == gen2);
   gen2();
   BOOST_CHECK(gen != gen2);
   //
   // construction and seeding:
   //
   engine_type gen3(0);
   gen3.seed(2);
}

BOOST_AUTO_TEST_CASE(xor_combine_test)
{
   //
   // As above but with a few things missing which don't work - for example we have no 
   // way to drill down and get the seed-type of the underlying generator.
   //
   typedef boost::random::xor_combine_engine<boost::random::independent_bits_engine<boost::random::mt19937, 512, boost::multiprecision::uint1024_t >, 512, boost::random::independent_bits_engine<boost::random::mt19937, 512, boost::multiprecision::uint1024_t >, 10> engine_type;
   typedef engine_type::result_type test_type;

   engine_type gen;
   gen.seed();
   test_type a = gen.min();
   test_type b = gen.max();
   BOOST_CHECK(a < b);
   a = gen();
   gen.generate(&b, &b + 1);
   gen.discard(20);

   //typename engine_type::base_type base(gen.base());

   std::stringstream ss;
   ss << gen;
   engine_type gen2;
   ss >> gen2;
   BOOST_CHECK(gen == gen2);
   gen2();
   BOOST_CHECK(gen != gen2);
   //
   // construction and seeding:
   //
   //engine_type gen3(0);
   //gen3.seed(2);
}

typedef boost::mpl::list <
   boost::random::random_number_generator<boost::random::mt19937, boost::multiprecision::cpp_int>,
   boost::random::random_number_generator<boost::random::mt19937, boost::multiprecision::uint1024_t>,
   boost::random::random_number_generator<boost::random::mt19937, boost::multiprecision::checked_uint1024_t>
> generators;


BOOST_AUTO_TEST_CASE_TEMPLATE(random_number_generator, generator_type, generators)
{
   typedef typename generator_type::result_type result_type;
   typedef typename generator_type::base_type base_type;

   result_type lim = 1;
   lim <<= 500;

   base_type base;
   generator_type gen(base);

   for(unsigned i = 0; i < 100; ++i)
      BOOST_CHECK(gen(lim) < lim);
}


