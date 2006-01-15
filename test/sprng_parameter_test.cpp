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

#include <boost/test/test_tools.hpp>
#include <boost/test/included/test_exec_monitor.hpp>


template<class PRNG>
void test(const std::string & name, const PRNG &)
{
  using namespace boost::random;
  std::cout << "Testing " << name << ": ";
  PRNG rng;  // default ctor
  for(int i = 0; i < 9999; i++)
    rng();
  typename PRNG::result_type val = rng();
  
  PRNG rng2(stream_number=0, global_seed=0, parameter=0, total_streams=1);
  for(int i = 0; i < 9999; i++)
    rng2();
  typename PRNG::result_type val2 = rng2();

  PRNG rng3(stream_number=1, total_streams=2);
  for(int i = 0; i < 9999; i++)
    rng3();
  typename PRNG::result_type val3 = rng3();

  PRNG rng4;
  rng4.seed_implementation(1,2);
  for(int i = 0; i < 9999; i++)
    rng4();
  typename PRNG::result_type val4 = rng4();
  
  bool result = (val==val2) && (val != val3) && (val3==val4);
  std::cout << val << " " << val2 << " " << val3 << " " << val4 <<  std::endl;
  BOOST_CHECK(result);
}

void test_all()
{
  test("sprng::lcg64", boost::random::sprng::lcg64());
}


int test_main(int, char*[])
{

  test_all();
  return 0;
}
