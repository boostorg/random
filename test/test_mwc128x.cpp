/* test_mwc128x.cpp
 *
 * Copyright Sebastiano Vigna 2026
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include <boost/random/mwcx.hpp>
#include <cstdint>

#define BOOST_RANDOM_URNG boost::random::mwc128x
#define BOOST_RANDOM_CPP11_URNG
#define BOOST_RANDOM_PROVIDED_SEED_TYPE std::uint64_t

// Values computed with the reference implementation (https://github.com/vigna/MWCPRNG)
// from the initial states produced by the default constructor and by seed_seq
#define BOOST_RANDOM_VALIDATION_VALUE UINT64_C(0x420e82af9a315547)
#define BOOST_RANDOM_SEED_SEQ_VALIDATION_VALUE UINT64_C(0x3478cb0803b88bde)

// Larger than the distance below which discard() steps the generator
#define BOOST_RANDOM_DISCARD_COUNT2 1000000

#include "test_generator.ipp"
