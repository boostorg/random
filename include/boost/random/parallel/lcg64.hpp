/* boost random/parallel/lcg64.hpp header file
 *
 * Copyright Matthias Troyer 2006
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef BOOST_RANDOM_PARALLEL_LCG64_HPP
#define BOOST_RANDOM_PARALLEL_LCG64_HPP

#include <iostream>
#include <cassert>
#include <stdexcept>
#include <boost/config.hpp>
#include <boost/limits.hpp>
#include <boost/static_assert.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/random/parallel/keyword.hpp>
#include <boost/random/parallel/detail/get_prime.hpp>
#include <boost/random/parallel/detail/seed_macros.hpp>
#include <boost/parameter/macros.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/assert.hpp>
#include <boost/integer_traits.hpp>

#if !defined(BOOST_NO_INT64_T) && !defined(BOOST_NO_INTEGRAL_INT64_T)

namespace boost {
namespace random {
namespace parallel {

// 64-bit parallel lineacr congruential generator, 
// following the SPRNG implementation

template<uint64_t a, uint64_t val>
class lcg64
{
public:
  typedef uint64_t result_type;
#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION
  static const bool has_fixed_range = true;
  static const result_type min_value = 0;
  static const result_type max_value =integer_traits<uint64_t>::const_max;
  static const result_type max_streams = 146138719;
#else
  BOOST_STATIC_CONSTANT(bool, has_fixed_range = false);
  BOOST_STATIC_CONSTANT(result_type, max_streams = 146138719);
#endif

/*
// forward seeding functions with iterator buffers to named versions
#define BOOST_LCG64_SEED_IT(z, n, unused)                                         \
  template <class It BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n,class T)>        \
  void seed(It& first, It const& last BOOST_PP_COMMA_IF(n)                        \
               BOOST_PP_ENUM_BINARY_PARAMS(n,T,const& x))                         \
  {                                                                               \
    if(first == last)                                                             \
      throw std::invalid_argument("boost::sprng::parallel::lcg64::seed");         \
    seed(global_seed=*first++ BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n,x) );   \
  }
   
BOOST_PP_REPEAT_FROM_TO(0, BOOST_RANDOM_MAXARITY, BOOST_LCG64_SEED_IT,~)

#undef BOOST_LCG64_SEED_IT
*/

  // forwarding named seeding functions
  BOOST_RANDOM_PARALLEL_SEED(lcg64)
  {
    unsigned int stream = p[stream_number|0u];
    unsigned int s=p[global_seed|0u];
    BOOST_ASSERT(stream < p[total_streams|1u]);
    BOOST_ASSERT(p[total_streams|1u] < max_streams);
    // seed the generator
    c = detail::get_prime_64(stream);
    _x =  (uint64_t(0x2bc6ffffU)<<32 | 0x8cfe166dU)^((uint64_t(s)<<33)|stream);
    // and advance it a bit
    for(uint64_t i=0; i<127*stream; i++)
      operator()();
  }

  BOOST_RANDOM_PARALLEL_ITERATOR_SEED_DEFAULT()

  result_type min BOOST_PREVENT_MACRO_SUBSTITUTION () const { return 0; }
  result_type max BOOST_PREVENT_MACRO_SUBSTITUTION () const { return std::numeric_limits<result_type>::max(); }

  result_type operator()()
  {
    _x = a * _x + c;
    return _x;
  }

  static bool validation(uint64_t x) { return x==val; }

#ifdef BOOST_NO_OPERATORS_IN_NAMESPACE
    
  // Use a member function; Streamable concept not supported.
  bool operator==(const lcg64& rhs) const
  { return _x == rhs._x && c==rhs.c; }
  bool operator!=(const lcg64& rhs) const
  { return !(*this == rhs); }

#else 
  friend bool operator==(const lcg64& x,
                         const lcg64& y)
  { return x._x == y._x && x.c == y.c; }
  friend bool operator!=(const lcg64& x,
                         const lcg64& y)
  { return !(x == y && x.c == y.c); }
    
#if !defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS) && !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x551))
  template<class CharT, class Traits>
  friend std::basic_ostream<CharT,Traits>&
  operator<<(std::basic_ostream<CharT,Traits>& os,
             const lcg64& lcg)
  {
    return os << lcg._x << " " << lcg.c;
  }

  template<class CharT, class Traits>
  friend std::basic_istream<CharT,Traits>&
  operator>>(std::basic_istream<CharT,Traits>& is,
             lcg64& lcg)
  {
    return is >> lcg._x >> lcg.c;
  }
 
private:
#endif
#endif
    
  uint64_t _x;
  uint64_t c;
};

#if defined(BOOST_NO_OPERATORS_IN_NAMESPACE) || defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS) || BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x551))
template<class CharT, class Traits, uint64_t a, unit64_t val>
std::basic_ostream<CharT,Traits>&
operator<<(std::basic_ostream<CharT,Traits>& os,
           const lcg64<a,val>& lcg)
{
    return os << lcg._x << " " << lcg.c;
}

template<class CharT, class Traits, uint64_t a, unit64_t val>
std::basic_istream<CharT,Traits>&
operator>>(std::basic_istream<CharT,Traits>& is,
           lcg64<a,val>& lcg)
{
    return is >> lcg._x >> lcg.c;
}
#endif

#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION
//  A definition is required even for integral static constants
template<uint64_t a, uint64_t val>
const bool lcg64<a,val>::has_fixed_range;
template<uint64_t a, uint64_t val>
const uint64_t lcg64<a,val>::min_value;
template<uint64_t a, uint64_t val>
const uint64_t lcg64<a,val>::max_value;
template<uint64_t a, uint64_t val>
const uint64_t lcg64<a,val>::max_streams;
#endif

} } // namespace random::parallel

// the three tested versions, validation still missing

typedef random::parallel::lcg64<uint64_t(0x87b0b0fdU)|uint64_t(0x27bb2ee6U)<<32,
                      uint64_t( 481823773Ul)+(uint64_t(3380683238Ul)<<32)> lcg64a;
typedef random::parallel::lcg64<uint64_t(0xe78b6955U)|uint64_t(0x2c6fe96eU)<<32,
                      uint64_t(3274024413Ul)+(uint64_t(3475904802Ul)<<32)> lcg64b;
typedef random::parallel::lcg64<uint64_t(0x31a53f85U)|uint64_t(0x369dea0fU)<<32,
                      uint64_t( 950651229Ul)+(uint64_t(3996309981Ul)<<32)> lcg64c;

} // namespace boost

#endif // !BOOST_NO_INT64_T && !BOOST_NO_INTEGRAL_INT64_T
#endif // BOOST_RANDOM_LINEAR_CONGRUENTIAL_HPP
