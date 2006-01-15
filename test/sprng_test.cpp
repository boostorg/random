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
#include <boost/random/sprng.hpp>

#include <boost/test/test_tools.hpp>
#include <boost/test/included/test_exec_monitor.hpp>

#ifdef BOOST_NO_STDC_NAMESPACE
  namespace std { using ::abs; using ::fabs; using ::pow; }
#endif

/*
 * Validate correct implementation
 */

// validation value not yet obtained
template <class RNG>
bool check(double x, const RNG&) { return true; }

template<class PRNG>
void validate(const std::string & name, const PRNG &)
{
  std::cout << "validating " << name << ": ";
  PRNG rng;  // default ctor
  for(int i = 0; i < 9999; i++)
    rng();
  typename PRNG::result_type val = rng();
  // make sure the validation function is a static member
  bool result = check(val, rng);
  
  // allow for a simple eyeball check for MSVC instantiation brokenness
  // (if the numbers for all generators are the same, it's obviously broken)
  std::cout << val << " " << rng() << std::endl;
  BOOST_CHECK(result);
}

void validate_all()
{
  using namespace boost;
  validate("sprng::lfg", boost::random::sprng::lfg());
  validate("sprng::lcg", boost::random::sprng::lcg());
  validate("sprng::lcg64", boost::random::sprng::lcg64());
  validate("sprng::cmrg", boost::random::sprng::cmrg());
  validate("sprng::mlfg", boost::random::sprng::mlfg());
//  validate("sprng::pmlcg", boost::random::sprng::pmlcg());
}


/*
 * Check function signatures
 */

#if BOOST_WORKAROUND( __BORLANDC__, BOOST_TESTED_AT( 0x570) )
#pragma warn -par
#endif
template<class URNG, class Dist>
void instantiate_dist(URNG& urng, const char * name, const Dist& dist)
{
  // this makes a copy of urng
  boost::variate_generator<URNG, Dist> gen(urng, dist);

  // this keeps a reference to urng
  boost::variate_generator<URNG&, Dist> genref(urng, dist);

#ifndef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
  // and here is a pointer to (a copy of) the urng
  URNG copy = urng;
  boost::variate_generator<URNG*, Dist> genptr(&copy, dist);
#endif

  for(int i = 0; i < 1000; ++i) {
    (void) gen();
    (void) genref();
#ifndef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
    (void) genptr();
#endif
  }
  typename Dist::result_type g = gen();
  BOOST_CHECK(std::abs(g - genref()) < 1e-6);
#ifndef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
  BOOST_CHECK(std::abs(g - genptr()) < 1e-6);
#endif

  (void) gen.engine();
  gen.distribution().reset();

  Dist d = dist;            // copy ctor
  d = dist;                 // copy assignment

#if !defined(BOOST_NO_OPERATORS_IN_NAMESPACE) && !defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
  {
    std::ostringstream file;
    file << urng << std::endl;
    file << d;
    std::istringstream input(file.str());
    // std::cout << file.str() << std::endl;
    URNG restored_engine;
    input >> restored_engine;
    input >> std::ws;
    Dist restored_dist;
    input >> restored_dist;
#if !defined(BOOST_MSVC) || BOOST_MSVC > 1300 // MSVC brokenness
    boost::variate_generator<URNG, Dist> old(urng, d);
    boost::variate_generator<URNG, Dist> restored(restored_engine, restored_dist);
    // advance some more so that state is exercised
    for(int i = 0; i < 1000; ++i) {
      (void) old();
      (void) restored();
    }
    BOOST_CHECK_MESSAGE((std::abs(old()-restored()) < 0.0001),
                        (std::string(name) + " old == restored_dist"));
#endif // BOOST_MSVC
  }
#endif // BOOST_NO_OPERATORS_IN_NAMESPACE
}

template<class URNG, class RealType>
void instantiate_real_dist(URNG& urng, RealType /* ignored */)
{
  instantiate_dist(urng, "uniform_real",
                   boost::uniform_real<RealType>(0, 2.1));
  instantiate_dist(urng, "triangle_distribution",
                   boost::triangle_distribution<RealType>(1, 1.5, 7));
  instantiate_dist(urng, "exponential_distribution",
                   boost::exponential_distribution<RealType>(5));
  instantiate_dist(urng, "normal_distribution",
                   boost::normal_distribution<RealType>());
  instantiate_dist(urng, "lognormal_distribution",
                   boost::lognormal_distribution<RealType>(1, 1));
  instantiate_dist(urng, "cauchy_distribution",
                   boost::cauchy_distribution<RealType>(1));
  instantiate_dist(urng, "gamma_distribution",
                   boost::gamma_distribution<RealType>(1));
}

template<class URNG, class ResultType>
void instantiate_urng(const std::string & s, const URNG &, const ResultType &)
{
  std::cout << "Basic tests for " << s;
  URNG urng;
  urng.seed();                                  // seed() member function
  int a[URNG::has_fixed_range ? 5 : 10];        // compile-time constant
  (void) a;   // avoid "unused" warning
  typename URNG::result_type x1 = urng();
  ResultType x2 = x1;
  (void) &x2;           // avoid "unused" warning

  URNG urng2 = urng;             // copy constructor
#if !defined(BOOST_MSVC) || BOOST_MSVC > 1300 // MSVC brokenness
  BOOST_CHECK(urng == urng2);     // operator==
  BOOST_CHECK(!(urng != urng2));  // operator!=
  urng();
  urng2 = urng;                  // copy assignment
  BOOST_CHECK(urng == urng2);
#endif // BOOST_MSVC

  const std::vector<int> v(9999u, 0x41);
  std::vector<int>::const_iterator it = v.begin();
  std::vector<int>::const_iterator it_end = v.end();
  URNG urng3(it, it_end);
  BOOST_CHECK(it != v.begin());
  std::cout << "; seeding uses " << (it - v.begin()) << " words" << std::endl;

  bool have_exception = false;
  try {
    // now check that exceptions are thrown
    it = v.end();
    urng3.seed(it, it_end);
  } catch(std::invalid_argument& x) {
    have_exception = true;
  }
  BOOST_CHECK(have_exception);

  // check for min/max members
  ResultType min = (urng3.min)();
  (void) &min;
  ResultType max = (urng3.max)();
  (void) &max;

#if !defined(BOOST_NO_OPERATORS_IN_NAMESPACE) && !defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
  // Streamable concept not supported for broken compilers

  // advance a little so that state is relatively arbitrary
  for(int i = 0; i < 9307; ++i)
    urng();
  urng2 = urng;

  {
    // narrow stream first
    std::ostringstream file;
    file << urng;
    // move forward
    urng();
    // restore old state
    std::istringstream input(file.str());
    input >> urng;
    // std::cout << file.str() << std::endl;
#if !defined(BOOST_MSVC) || BOOST_MSVC > 1300 // MSVC brokenness
    // advance some more so that state is exercised
    for(int i = 0; i < 10000; ++i) {
      urng();
      urng2();
    }
    BOOST_CHECK(urng == urng2);
#endif // BOOST_MSVC
  }
  
  urng2 = urng;
#if !defined(BOOST_NO_STD_WSTREAMBUF) && !defined(BOOST_NO_STD_WSTRING)
  {
    // then wide stream
    std::wostringstream file;
    file << urng;
    // move forward
    urng();
    std::wistringstream input(file.str());
    input >> urng;
#if !defined(BOOST_MSVC) || BOOST_MSVC > 1300 // MSVC brokenness
    // advance some more so that state is exercised
    for(int i = 0; i < 10000; ++i) {
      urng();
      urng2();
    }
    BOOST_CHECK(urng == urng2);
#endif // BOOST_MSVC
  }
#endif // BOOST_NO_STD_WSTREAMBUF, BOOST_NO_STD_WSTRING
#endif // BOOST_NO_OPERATORS_IN_NAMESPACE etc.

  // instantiate various distributions with this URNG
  // instantiate_dist(urng, "uniform_smallint", boost::uniform_smallint(0, 11));
  instantiate_dist(urng, "uniform_int", boost::uniform_int<>(-200, 20000));
  instantiate_dist(urng, "bernoulli_distribution",
                   boost::bernoulli_distribution<>(0.2));
  instantiate_dist(urng, "binomial_distribution",
                   boost::binomial_distribution<>(4, 0.2));
  instantiate_dist(urng, "geometric_distribution",
                   boost::geometric_distribution<>(0.8));
  instantiate_dist(urng, "poisson_distribution",
                   boost::poisson_distribution<>(1));

  instantiate_real_dist(urng, 1.0f);
  instantiate_real_dist(urng, 1.0);
  instantiate_real_dist(urng, 1.0l);

#if 0
  // We cannot compare the outcomes before/after save with std::abs(x-y)
  instantiate_dist("uniform_on_sphere",
                   boost::uniform_on_sphere<URNG>(urng, 2));
#endif
}

void instantiate_all()
{
  using namespace boost;
  instantiate_urng("sprng::lfg", boost::random::sprng::lfg(),0.);
  instantiate_urng("sprng::lcg", boost::random::sprng::lcg(),0.);
  instantiate_urng("sprng::lcg64", boost::random::sprng::lcg64(),0.);
  instantiate_urng("sprng::cmrg", boost::random::sprng::cmrg(),0.);
  instantiate_urng("sprng::mlfg", boost::random::sprng::mlfg(),0.);
//  instantiate_urng("sprng::pmlcg", boost::random::sprng::pmlcg(),0.);
}


#if defined(BOOST_MSVC) && _MSC_VER < 1300

// These explicit instantiations are necessary, otherwise MSVC does
// not find the <boost/operators.hpp> inline friends.
// We ease the typing with a suitable preprocessor macro.
#define INSTANT(x) \
template class boost::uniform_smallint<x>; \
template class boost::uniform_int<x>; \
template class boost::uniform_real<x>; \
template class boost::bernoulli_distribution<x>; \
template class boost::geometric_distribution<x>; \
template class boost::triangle_distribution<x>; \
template class boost::exponential_distribution<x>; \
template class boost::normal_distribution<x>; \
template class boost::uniform_on_sphere<x>; \
template class boost::lognormal_distribution<x>;

INSTANT(boost::random::sprng::lfg)
INSTANT(boost::random::sprng::lcg)
INSTANT(boost::random::sprng::lcg64)
INSTANT(boost::random::sprng::cmrg)
INSTANT(boost::random::sprng::mlfg)
//INSTANT(boost::random::sprng::pmlcg)

#undef INSTANT
#endif


int test_main(int, char*[])
{

#if !defined(__INTEL_COMPILER) || !defined(_MSC_VER) || __INTEL_COMPILER > 700 
  instantiate_all();
  validate_all();

  return 0;
#else
  std::cout << "Intel 7.00 on Win32 loops, so the test is disabled\n";
  return 1;
#endif
}
