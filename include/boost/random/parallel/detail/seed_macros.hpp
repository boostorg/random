/* boost random/parallel/lcg64.hpp header file
 *
 * Copyright Matthias Troyer 2006
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef BOOST_RANDOM_PARALLEL_DETAIL_SEED_MACROS_HPP
#define BOOST_RANDOM_PARALLEL_DETAIL_SEED_MACROS_HPP

#include <boost/random/parallel/keyword.hpp>
#include <boost/parameter/macros.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/repeat_from_to.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/array/elem.hpp>

#define BOOST_RANDOM_PARALLEL_CONSTRUCTOR(z, n, P)                          \
  BOOST_PP_IF(n,template <, BOOST_PP_EMPTY())                               \
  BOOST_PP_ENUM_PARAMS(n,class T) BOOST_PP_IF(n,>, BOOST_PP_EMPTY() )       \
  BOOST_PP_ARRAY_ELEM(0,P) (BOOST_PP_ENUM_BINARY_PARAMS(n,T,const& x))      \
  BOOST_PP_ARRAY_ELEM(1,P)                                                  \
  {                                                                         \
    seed(BOOST_PP_ENUM_PARAMS(n,x) );                                       \
  }                                                                         \
                                                                            \
  template <class It BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n,class T)>  \
  BOOST_PP_ARRAY_ELEM(0,P) (It& f, It const& l BOOST_PP_COMMA_IF(n)         \
          BOOST_PP_ENUM_BINARY_PARAMS(n,T,const& x))                        \
  BOOST_PP_ARRAY_ELEM(1,P)                                                  \
  {                                                                         \
    seed(f,l BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n,x) );              \
  }                                                                         \
                                                                            \
  BOOST_PP_IF(n,template <, BOOST_PP_EMPTY())                               \
  BOOST_PP_ENUM_PARAMS(n,class T) BOOST_PP_IF(n,>, BOOST_PP_EMPTY())        \
  void seed(BOOST_PP_ENUM_BINARY_PARAMS(n,T,const& x))                      \
  {                                                                         \
    seed_named(BOOST_PP_ENUM_PARAMS(n,x) );                                 \
  }

#define BOOST_RANDOM_PARALLEL_ITERATOR_SEED_IMPL(z, n, unused)              \
  template <class It BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n,class T)>  \
  void seed(It& f, It const& l BOOST_PP_COMMA_IF(n)                         \
               BOOST_PP_ENUM_BINARY_PARAMS(n,T,const& x))                   \
  {                                                                         \
    if(f == l)                                                              \
      throw std::invalid_argument("invalid seeding argument");              \
    seed(global_seed=*f++ BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n,x) ); \
  }
   


#define BOOST_RANDOM_PARALLEL_SEED_PARAMS(RNG,PARAMS,INIT)                                            \
  BOOST_PP_REPEAT_FROM_TO(0, BOOST_RANDOM_MAXARITY, BOOST_RANDOM_PARALLEL_CONSTRUCTOR,(2,(RNG,INIT))) \
  BOOST_PARAMETER_MEMFUN(void,seed_named,0,BOOST_RANDOM_MAXARITY,PARAMS)     

#define BOOST_RANDOM_PARALLEL_SEED(RNG) \
  BOOST_RANDOM_PARALLEL_SEED_PARAMS(RNG, boost::random::parallel_seed_params,BOOST_PP_EMPTY()) 


#define BOOST_RANDOM_PARALLEL_ITERATOR_SEED_DEFAULT()                                             \
  BOOST_PP_REPEAT_FROM_TO(0, BOOST_RANDOM_MAXARITY, BOOST_RANDOM_PARALLEL_ITERATOR_SEED_IMPL,~)

#endif // BOOST_RANDOM_PARALLEL_DETAIL_SEED_MACROS_HPP
