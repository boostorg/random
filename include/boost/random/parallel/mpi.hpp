/* 
 * Copyright Matthias Troyer 2006
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
*
 */


#ifndef BOOST_RANDOM_PARALLEL_MPI_HPP
#define BOOST_RANDOM_PARALLEL_MPI_HPP

#include <boost/random/parallel/seed.hpp>
#include <boost/parallel/mpi/collectives.hpp>
#include <boost/parallel/mpi/communicator.hpp>

namespace boost { namespace random { namespace parallel {
  
  template <class PRNG>
  void seed(
          PRNG& prng
        , boost::parallel::mpi::communicator const& c
      )
  {
    seed(prng, c.rank(), c.size());
  }

  template <class PRNG, class SeedType>
  void seed(
          PRNG& prng
        , boost::parallel::mpi::communicator const& c
        , SeedType const& s
      )
  {
    seed(prng, c.rank(), c.size(), s);
  }

  template <class PRNG, class Iterator>
  void seed(
          PRNG& prng
        , boost::parallel::mpi::communicator const& c
        , Iterator& first
        , Iterator const& last
      )
  {
    seed(prng, c.rank(), c.size(), first, last);
  }

  template <class PRNG, class SeedType>
  void broadcast_seed(
          PRNG& prng
        , boost::parallel::mpi::communicator const& c
        , int root
        , SeedType s
      )
  {
    boost::parallel::mpi::broadcast(c,s,root);
    seed(prng, c.rank(), c.size(), s);
  }

  template <class PRNG, class Iterator>
  void broadcast_seed(
          PRNG& prng
        , boost::parallel::mpi::communicator const& c
        , int root
        , Iterator& first
        , Iterator const& last
      )
  {
    // read a seed value only if I'm the root rank
    typename std::iterator_traits<Iterator>::value_type seed;
    if (c.rank()==root) {
      if(first == last)
        throw_exception(std::invalid_argument("parallel_seed"));
      seed = *first++;
    }
    broadcast_seed(prng, c, root, seed);
  }

} } } // namespace boost::random::parallel

#endif // BOOST_RANDOM_PARALLEL_MPI_HPP
