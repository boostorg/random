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

#ifndef BOOST_RANDOM_MAXARITY
#define BOOST_RANDOM_MAXARITY 5
#endif

namespace boost { namespace random { namespace parallel {
  using mpl::placeholders::_;
  
  BOOST_PARAMETER_KEYWORD(random_tag,stream_number)
  BOOST_PARAMETER_KEYWORD(random_tag,total_streams)
  BOOST_PARAMETER_KEYWORD(random_tag,global_seed)
  BOOST_PARAMETER_KEYWORD(random_tag,first)
  BOOST_PARAMETER_KEYWORD(random_tag,last)

  typedef parameter::parameters<
      parameter::optional<random_tag::stream_number, is_convertible<_,unsigned int> >
    , parameter::optional<random_tag::total_streams, is_convertible<_,unsigned int> >
    , parameter::optional<random_tag::global_seed>
  > seed_params;

  typedef parameter::parameters<
      parameter::optional<random_tag::stream_number, is_convertible<_,unsigned int> >
    , parameter::optional<random_tag::total_streams, is_convertible<_,unsigned int> >
    , parameter::required<random_tag::first>
    , parameter::required<random_tag::last>
  > iterator_seed_params;

} } } // namespace boost::random::parallel

#endif // BOOST_RANDOM_PARALLEL_KEYWORD_HPP
