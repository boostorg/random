/* boost random_test.cpp various tests
 *
 * Copyright Jens Maurer 2000
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * $Id$
 */

#include <boost/random/parallel/mpi.hpp>


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
#include <boost/random/parallel.hpp>
#include <boost/random/parallel/mpi.hpp>
#include <boost/parallel/mpi/environment.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/test/included/test_exec_monitor.hpp>


template<class PRNG>
void test(const std::string & name, const PRNG &)
{
  using namespace boost::random;
  
  boost::parallel::mpi::communicator comm;
  
  std::cout << "Testing " << name << ": ";
  PRNG rng;  // default ctor
  for(int i = 0; i < 10000; i++)
    rng();
  typename PRNG::result_type val = rng();
  
  PRNG rng2;
  parallel::seed(rng2,comm,0);
  for(int i = 0; i < 10000; i++)
    rng2();
  typename PRNG::result_type val2 = rng2();

  PRNG rng3;
  parallel::broadcast_seed(rng3,comm,0,0);
  for(int i = 0; i < 10000; i++)
    rng3();
  typename PRNG::result_type val3 = rng3();

  bool result = (val==val2) && (val == val3);
  std::cout << val << " " << val2 << " " << val3 <<  std::endl;
  BOOST_CHECK(result);
}

void test_all()
{
  test("lcg64a", boost::lcg64a());
}


int test_main(int argc, char** argv)
{
  boost::parallel::mpi::environment env(argc, argv);
  test_all();
  return 0;
}
