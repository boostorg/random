/* boost random/detail/random_device_file header file
*
* Copyright Jens Maurer 2000
* Copyright 2007 Andy Tompkins.
* Copyright Steven Watanabe 2010-2011
* Copyright 2017 James E. King, III
*
* Distributed under the Boost Software License, Version 1.0. (See
* accompanying file LICENSE_1_0.txt or copy at
* http://www.boost.org/LICENSE_1_0.txt)
*
* $Id$
*
* Revision history
*  2017-09-14  urandom implementation moved here
*/

#ifndef BOOST_RANDOM_DETAIL_RANDOM_DEVICE_FILE
#define BOOST_RANDOM_DETAIL_RANDOM_DEVICE_FILE

#include <boost/config.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if !defined(BOOST_WINDOWS)

#include <boost/core/ignore_unused.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/system_error.hpp>
#include <boost/throw_exception.hpp>
#include <cerrno>
#include <fcntl.h>    // open
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#if defined(BOOST_HAS_UNISTD_H)
#include <unistd.h>
#endif

namespace boost {
namespace random {
namespace detail {

template<class Entropy>
class random_device_file : private noncopyable
{
public:
    typedef Entropy result_type;

    random_device_file(const std::string& token = std::string())
        : fd_(0)
    {
        fd_ = open(token.empty() ? "/dev/urandom" : token.c_str(), O_RDONLY);

        if (-1 == fd_)
        {
            BOOST_THROW_EXCEPTION(system::system_error(
                errno, system::system_category(), "open " + std::string(token.empty() ? "/dev/urandom" : token.c_str())));
        }
    }

    ~random_device_file() BOOST_NOEXCEPT
    {
        if (fd_)
        {
            ignore_unused(close(fd_));
        }
    }

    result_type operator()()
    {
        result_type result;
        size_t offset = 0;
        do
        {
            ssize_t sz = read(fd_, reinterpret_cast<char *>(&result) + offset, sizeof(result) - offset);

            if (sz < 1)
            {
                BOOST_THROW_EXCEPTION(system::system_error(
                    errno, system::system_category(), "read"));
            }
            offset += sz;
        } while (offset < sizeof(result));

        return result;
    }

private:
    int fd_;
};

} // detail
} // random
} // boost

#endif // !defined(BOOST_WINDOWS)
#endif // BOOST_RANDOM_DETAIL_RANDOM_DEVICE_FILE
