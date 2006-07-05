/* boost random_test.cpp various tests
 *
 * Copyright Matthias Troyer 2006
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * $Id$
 */

#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <iterator>
#include <vector>
#include <boost/random.hpp>
#include <boost/config.hpp>
#include <boost/random/parallel/lcg64.hpp>
#include <boost/random/parallel/well.hpp>
#include <boost/random/buffered_generator.hpp>
#include <boost/random/buffered_uniform_01.hpp>

#include <boost/test/test_tools.hpp>
#include <boost/test/included/test_exec_monitor.hpp>

void simulate_gauss_impl(boost::buffered_generator<double>& gen)
{
  double sum=0;
  for (int i=0;i<100000;i++)
    sum += gen();
  std::cout << "The sum of Gaussians is " << sum << "\n";
}

void simulate_impl(boost::buffered_uniform_01<double>& gen)
{
  double sum=0;
  for (int i=0;i<100000;i++)
    sum += gen();
  typedef boost::variate_generator<boost::buffered_uniform_01<double>&,boost::normal_distribution<> > gen_type;
  gen_type gauss(gen,boost::normal_distribution<>());
  for (int i=0;i<100000;i++)
    sum += gauss();
  std::cout << "The sum of Uniforms and Gaussians is " << sum << "\n";
}



template <class RNG>
void simulate_gauss()
{
  typedef boost::variate_generator<RNG,boost::normal_distribution<> > gen_type;
  RNG engine;
  
boost::basic_buffered_generator<gen_type,double> gen(gen_type(engine,boost::normal_distribution<>()));
  simulate_gauss_impl(gen);
}

template <class RNG>
void simulate()
{
  boost::basic_buffered_uniform_01<RNG> gen;
  simulate_impl(gen);
}



template<class PRNG>
void test(const std::string & name, const PRNG &)
{
  using namespace boost::random;
  std::cout << "Testing " << name << ": ";
  PRNG rng;  // default ctor
  simulate<PRNG>();
  simulate_gauss<PRNG>();
}

void test_all()
{
  test("well512a", boost::well512a());
  test("lcg64a", boost::lcg64a());
}


int test_main(int, char*[])
{

  test_all();
  return 0;
}
