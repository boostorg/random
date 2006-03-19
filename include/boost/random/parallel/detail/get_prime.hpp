/* 
 * Copyright Matthias Troyer 2006
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
*
 */

#ifndef BOOST_RANDOM_PARALLEL_DETAIL_GET_PRIME_HPP
#define BOOST_RANDOM_PARALLEL_DETAIL_GET_PRIME_HPP

#include <boost/cstdint.hpp>

namespace boost { namespace random { namespace detail {

// get a prime number to be used as additive constant in a 64-bit LCG generator
uint64_t get_prime_64(unsigned int);

} } } // namespace boost::random::detail

#endif // BOOST_RANDOM_PARALLEL_DETAIL_GET_PRIME_HPP
