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
#include <boost/timer.hpp>
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
  for (int i=0;i<10000000;i++)
    sum += gen();
  std::cout << "The sum of Gaussians is " << sum << "\n";
}

void simulate_impl(boost::buffered_uniform_01<double>& gen)
{
  double sum=0;
  for (int i=0;i<10000000;i++)
    sum += gen();
  typedef boost::variate_generator<boost::buffered_uniform_01<double>&,boost::normal_distribution<> > gen_type;
  gen_type gauss(gen,boost::normal_distribution<>());
  for (int i=0;i<10000000;i++)
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

template <class RNG>
void simulate_direct_impl(RNG& gen)
{
  double sum=0;
  typedef boost::variate_generator<RNG&,boost::uniform_real<> > gen_type1;
  gen_type1 uniform(gen,boost::uniform_real<>());
  for (int i=0;i<10000000;i++)
    sum += gen();
  typedef boost::variate_generator<RNG&,boost::normal_distribution<> > gen_type2;
  gen_type2 gauss(gen,boost::normal_distribution<>());
  for (int i=0;i<10000000;i++)
    sum += gauss();
  std::cout << "The sum of Uniforms and Gaussians is " << sum << "\n";
}

template <class RNG>
void simulate_gauss_direct_impl(RNG& gen)
{
  double sum=0;
  for (int i=0;i<10000000;i++)
    sum += gen();
  std::cout << "The sum of Gaussians is " << sum << "\n";
}

template <class RNG>
void simulate_gauss_direct()
{
  typedef boost::variate_generator<RNG,boost::normal_distribution<> > gen_type;
  RNG engine;
  
  gen_type gen(engine,boost::normal_distribution<>());
  simulate_gauss_direct_impl(gen);
}

template <class RNG>
void simulate_direct()
{
  RNG engine;
  simulate_direct_impl(engine);
}



template<class PRNG>
void test(const std::string & name, const PRNG &)
{
  using namespace boost::random;
  std::cout << "Timing buffered  " << name << ":\n";
  boost::timer t;
  simulate<PRNG>();
  simulate_gauss<PRNG>();
  std::cout << "Time: " << t.elapsed() << "\n\n\n";
  std::cout << "Timing direct  " << name << ":\n";
  t.restart();
  simulate_direct<PRNG>();
  simulate_gauss_direct<PRNG>();
  std::cout << "Time: " << t.elapsed() << "\n\n\n";
}

void test_all()
{
  test("well512a", boost::well512a());
  test("lcg64a", boost::lcg64a());
  test("mt19937", boost::mt19937());
}


int test_main(int, char*[])
{

  test_all();
  return 0;
}
