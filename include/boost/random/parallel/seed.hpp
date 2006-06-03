/* 
 * Copyright Matthias Troyer 2006
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
*
 */

#include <boost/random/parallel/keyword.hpp>
#include <boost/throw_exception.hpp>
#include <stdexcept>
#include <boost/throw_exception.hpp>

#ifndef BOOST_RANDOM_PARALLEL_SEED_HPP
#define BOOST_RANDOM_PARALLEL_SEED_HPP

namespace boost { namespace random { namespace parallel {
  
  template <class PRNG>
  void seed(
          PRNG& prng
        , unsigned int num
        , unsigned int total
        , unsigned int seed=0u
      )
  {
    prng.seed(global_seed=seed, stream_number=num, total_streams=total);
  }

  template <class PRNG, class Iterator>
  void seed(
          PRNG& prng
        , unsigned int num
        , unsigned int total
        , Iterator& first
        , Iterator const& last
      )
  {
   if(first == last)
      throw_exception(std::invalid_argument("parallel_seed"));
    seed(prng, num, total, *first++);
  }


} } } // namespace boost::random::parallel

#endif // BOOST_RANDOM_PARALLEL_SEED_HPP
