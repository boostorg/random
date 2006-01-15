/* boost random_test.cpp various tests
 *
 * Copyright Jens Maurer 2000
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * $Id$
 */

#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
#pragma warning( disable : 4786 )
#endif

#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <iterator>
#include <vector>
#include <boost/random.hpp>
#include <boost/config.hpp>
#include <boost/random/sprng/lcg64.hpp>
#include <boost/random/buffered_uniform_01.hpp>

#include <boost/test/test_tools.hpp>
#include <boost/test/included/test_exec_monitor.hpp>

template <class ValueType>
ValueType forward(boost::buffered_generator<ValueType>& gen)
{
  for(int i = 0; i < 9999; i++)
    gen();
  return gen();
}


template<class PRNG>
void test(const std::string & name, const PRNG &)
{
  using namespace boost::random;
  std::cout << "Testing " << name << ": ";
  PRNG rng;  // default ctor
  PRNG rng_copy;  // default ctor
  typedef double result_type;
  
  boost::buffered_uniform_01<PRNG> gen_default;
  boost::buffered_uniform_01<PRNG> gen_copy(rng);
  boost::buffered_uniform_01<PRNG&> gen_ref(rng);
  
  result_type val_default=forward(gen_default);
  result_type val_copy=forward(gen_copy);
  result_type val_ref=forward(gen_ref);
  
  for(int i = 0; i < (10000/BOOST_BUFFERED_GENERATOR_BUFFER_SIZE+1)*BOOST_BUFFERED_GENERATOR_BUFFER_SIZE; i++)
    rng_copy();
  result_type val_ref_next = rng();
  result_type val_next = rng_copy();
  
  bool result = (val_default==val_copy) && (val_copy == val_ref) && (val_ref_next==val_next);
  std::cout << val_default << " " << val_copy << " " << val_ref << " " 
            << val_ref_next <<  " " << val_next << std::endl;
  BOOST_CHECK(result);
  
  gen_default.reset();
  gen_copy.reset();
  gen_ref.reset();
}

void test_all()
{
  test("minstd_rand", boost::minstd_rand());
  test("sprng::lcg64", boost::random::sprng::lcg64());
}


int test_main(int, char*[])
{

  test_all();
  return 0;
}
