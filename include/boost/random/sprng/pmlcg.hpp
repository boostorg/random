/* 
 * Copyright Matthias Troyer 2006
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
*
 */

#ifndef BOOST_RANDOM_SPRNG_PMLCG_HPP
#define BOOST_RANDOM_SPRNG_PMLCG_HPP


#define BOOST_SPRNG_GENERATOR   pmlcg
#define BOOST_SPRNG_TYPE        5
#define BOOST_SPRNG_MAX_STREAMS (1<<30)
#define BOOST_SPRNG_MAX_PARAMS  1

#include <boost/random/sprng/detail/implementation.hpp>

#undef BOOST_SPRNG_GENERATOR
#undef BOOST_SPRNG_TYPE
#undef BOOST_SPRNG_MAX_STREAMS
#undef BOOST_SPRNG_MAX_PARAMS

#endif // BOOST_RANDOM_SPRNG_PMLCG_HPP
