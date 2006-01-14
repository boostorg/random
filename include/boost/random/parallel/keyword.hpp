/* 
 * Copyright Matthias Troyer 2006
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
*
 */

#ifndef BOOST_RANDOM_PARALLEL_KEYWORD_HPP
#define BOOST_RANDOM_PARALLEL_KEYWORD_HPP

namespace boost { namespace random {

  BOOST_PARAMETER_KEYWORD(tag,stream_number)
  BOOST_PARAMETER_KEYWORD(tag,total_streams)
  BOOST_PARAMETER_KEYWORD(tag,global_seed)
  BOOST_PARAMETER_KEYWORD(tag,parameter)

  typedef parameter::parameters<
      parameter::optional<tag::stream_number, boost::is_convertible<_,unsigned int> >
    , parameter::optional<tag::total_streams, boost::is_convertible<_,unsigned int> >
    , parameter::optional<tag::global_seed, boost::is_convertible<_,int> >
    , parameter::optional<tag::parameter, boost::is_convertible<_,unsigned int> >
  > parallel_seed_params;

} // namespace boost::random

#endif // BOOST_RANDOM_PARALLEL_KEYWORD_HPP
