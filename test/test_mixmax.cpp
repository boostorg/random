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
#define BOOST_RANDOM_SEED_SEQ_VALIDATION_VALUE 1U
#define BOOST_RANDOM_ITERATOR_VALIDATION_VALUE 1U

// 10000th invocation with default constructor should give:
#define BOOST_RANDOM_VALIDATION_VALUE UINT64_C(545605927039957370)
#define BOOST_RANDOM_GENERATE_VALUES { 0x0U, 0x0U, 0x0U, 0x0U }

#include "test_generator.ipp"

struct seed_seq_0 {
    template<class It>
    void generate(It begin, It end) const {
        std::fill(begin, end, boost::uint32_t(0));
    }
};

struct seed_seq_1 {
    template<class It>
    void generate(It begin, It end) const {
        std::fill(begin, end, boost::uint32_t(0));
        *(end - 1) = 1;
    }
};

BOOST_AUTO_TEST_CASE(test_special_seed) {
    {
        seed_seq_1 seed;
        std::vector<boost::uint32_t> vec(624);
        seed.generate(vec.begin(), vec.end());
        
        std::vector<boost::uint32_t>::iterator it = vec.begin();
        boost::random::mixmax gen1(it, vec.end());
        BOOST_CHECK_EQUAL(gen1(), 0u);
        BOOST_CHECK_EQUAL(gen1(), 0u);
        
        boost::random::mixmax gen2(seed);
        BOOST_CHECK_EQUAL(gen2(), 0u);
        BOOST_CHECK_EQUAL(gen2(), 0u);
        
        BOOST_CHECK_EQUAL(gen1, gen2);
    }
    {
        seed_seq_0 seed;
        std::vector<boost::uint32_t> vec(61);
        seed.generate(vec.begin(), vec.end());
        
        std::vector<boost::uint32_t>::iterator it = vec.begin();
        boost::random::mixmax gen1(it, vec.end());
        BOOST_CHECK_EQUAL(gen1(), 1141379330u);
        BOOST_CHECK_EQUAL(gen1(), 0u);
        
        boost::random::mixmax gen2(seed);
        BOOST_CHECK_EQUAL(gen2(), 1141379330u);
        BOOST_CHECK_EQUAL(gen2(), 0u);
        
        BOOST_CHECK_EQUAL(gen1, gen2);
    }
}

int main(){return 0;}
