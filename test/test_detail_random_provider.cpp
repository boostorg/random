//
// Copyright (c) 2017 James E. King III
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//   http://www.boost.org/LICENCE_1_0.txt)
//
// Positive and negative testing for detail::random_provider
//

#include <boost/array.hpp>
#include <boost/cstdint.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/limits.hpp>
#include <string.h>

// The mock needs to load first for posix system call redirection to work properly
#include "mock_random.hpp"
#include <boost/random/random_device.hpp>


int main(int, char*[])
{
#if !defined(BOOST_RANDOM_TEST_RANDOM_MOCK)   // Positive Testing

    boost::random::detail::random_provider provider;

    // test get_random_bytes()
    char buf1[64];
    char buf2[64];
    provider.get_random_bytes(buf1, 64);
    provider.get_random_bytes(buf2, 64);
    BOOST_TEST_NE(0, memcmp(buf1, buf2, 64));

#else                                       // Negative Testing

    if (expectations_capable())
    {
        // Test fail acquiring context if the provider supports it
        if (provider_acquires_context())
        {
            expect_next_call_success(false);
            BOOST_TEST_THROWS(boost::random::detail::random_provider(), 
                              boost::random::entropy_error);
            BOOST_TEST(expectations_met());
        }

        // Test fail acquiring entropy
        if (provider_acquires_context())
        {
            expect_next_call_success(true);
        }
        expect_next_call_success(false);
        // 4 is important for the posix negative test (partial read) to work properly
        // as it sees a size of 4, returns 1, causing a 2nd loop to read 3 more bytes...
        char buf[4];
        BOOST_TEST_THROWS(boost::random::detail::random_provider().get_random_bytes(buf, 4), 
                          boost::random::entropy_error);
        BOOST_TEST(expectations_met());
    }

#endif

    return boost::report_errors();
}
