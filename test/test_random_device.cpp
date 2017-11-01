/* boost random_test.cpp various tests
 *
 * Copyright (c) 2010 Steven Watanabe
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENCE_1_0.txt)
 *
 * $Id$
 */

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <fstream>

#include <boost/random/random_device.hpp>
#include <boost/random/detail/random_device_file.hpp>

BOOST_AUTO_TEST_CASE(random_device)
{
    boost::random_device rng;

    BOOST_CHECK_EQUAL(0u, rng.min());
    BOOST_CHECK_EQUAL(~0u, rng.max());

    for (int i = 0; i < 100; ++i) 
    {
        boost::random_device::result_type val = rng();
        BOOST_CHECK_GE(val, 0u);
        BOOST_CHECK_LE(val, ~0u);
    }

    boost::random_device::result_type a[10];
    rng.generate(a, a + 10);
}

#if !defined(BOOST_WINDOWS)
BOOST_AUTO_TEST_CASE(random_device_file_not_there)
{
    using namespace boost::random;
    using namespace boost::system;

    BOOST_CHECK_THROW(
        basic_random_device<detail::random_device_file<unsigned char> >("__fictitious_and_nonexistent_filename__"),
        system_error
    );
}

BOOST_AUTO_TEST_CASE(random_device_file)
{
    using namespace boost::random;
    using namespace boost::system;

    const char *tmpfn = "random_device_file_entropy";
    int wfd = open(tmpfn, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    BOOST_REQUIRE_NE(wfd, -1);

    basic_random_device<detail::random_device_file<unsigned char> > rng(tmpfn);

    // Add some data to the entropy file
    BOOST_CHECK_EQUAL(2, write(wfd, "AB", 2));

    // We can read two characters before we run out of entropy from the file
    BOOST_CHECK_EQUAL(0x41, rng());
    BOOST_CHECK_EQUAL(0x42, rng());

    // Not enough entropy exists for another
    BOOST_CHECK_THROW(rng(), system_error);

    // clean up
    BOOST_CHECK_EQUAL(close(wfd), 0);
    BOOST_CHECK_EQUAL(unlink(tmpfn), 0);
}
#endif
