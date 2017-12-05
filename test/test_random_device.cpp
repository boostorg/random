/* boost random_test.cpp various tests
 *
 * Copyright (c) 2010 Steven Watanabe
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENCE_1_0.txt)
 *
 * $Id$
 */

#include <boost/random/random_device.hpp>
#include <boost/format.hpp>

#define BOOST_RANDOM_TEST_BUFSIZ 256

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_random_device)
{
    boost::random_device rng;

    BOOST_CHECK(typeid(boost::random_device::result_type) == typeid(unsigned int));

    BOOST_CHECK(!boost::random_device::has_fixed_range);

    BOOST_CHECK_EQUAL(rng.min(), 0u);
    BOOST_CHECK_EQUAL(rng.max(), ~0u);

    BOOST_CHECK_GT(rng.entropy(), 0);

    for (int i = 0; i < 100; ++i) {
        boost::random_device::result_type val = rng();
        BOOST_CHECK_GE(val, (rng.min)());
        BOOST_CHECK_LE(val, (rng.max)());
    }

    char buf[BOOST_RANDOM_TEST_BUFSIZ];
    char buf2[BOOST_RANDOM_TEST_BUFSIZ];
    rng.get_random_bytes(buf, BOOST_RANDOM_TEST_BUFSIZ);
    rng.get_random_bytes(buf2, BOOST_RANDOM_TEST_BUFSIZ);
    BOOST_CHECK_NE(0, ::memcmp(buf, buf2, BOOST_RANDOM_TEST_BUFSIZ));

    BOOST_TEST_MESSAGE(boost::format("entropy provider: %1%") % rng.name());

    boost::uint32_t a[10];
    rng.generate(a, a + 10);
}

