/* test_mixmax.cpp
 *
 * Copyright Kostas Savvidis 2019
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * $Id$
 *
 */

#include <boost/random/mixmax.hpp>
#include <cmath>

#define BOOST_RANDOM_URNG boost::random::mixmax

#define BOOST_RANDOM_SEED_WORDS 1

//#define BOOST_RANDOM_VALIDATION_VALUE 1U
//#define BOOST_RANDOM_SEED_SEQ_VALIDATION_VALUE 1U
//#define BOOST_RANDOM_ITERATOR_VALIDATION_VALUE 1U

// 10000th invocation with default constructor should give:
#define BOOST_RANDOM_VALIDATION_VALUE UINT64_C(545605927039957370)
//#define BOOST_RANDOM_GENERATE_VALUES { 0x0U, 0x0U, 0x0U, 0x0U }

#include "test_generator.ipp"
