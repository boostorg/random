/* 
 * Copyright Matthias Troyer 2006
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
*
 */

#include <boost/parameter/keyword.hpp>
#include <boost/parameter/parameters.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/mpl/placeholders.hpp>

#ifndef BOOST_RANDOM_PARALLEL_KEYWORD_HPP
#define BOOST_RANDOM_PARALLEL_KEYWORD_HPP

namespace boost { namespace random {
  using mpl::placeholders::_;
  
  BOOST_PARAMETER_KEYWORD(tag,stream_number)
  BOOST_PARAMETER_KEYWORD(tag,total_streams)
  BOOST_PARAMETER_KEYWORD(tag,global_seed)
  BOOST_PARAMETER_KEYWORD(tag,parameter)

  typedef parameter::parameters<
      parameter::optional<tag::stream_number, is_convertible<_,unsigned int> >
    , parameter::optional<tag::total_streams, is_convertible<_,unsigned int> >
    , parameter::optional<tag::global_seed, is_convertible<_,int> >
    , parameter::optional<tag::parameter, is_convertible<_,unsigned int> >
  > parallel_seed_params;

} } // namespace boost::random

#endif // BOOST_RANDOM_PARALLEL_KEYWORD_HPP
