/* boost random/detail/random_device_wincrypt header file
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
 *  2017-09-14  wincrypt implementation moved here
 */

#ifndef BOOST_RANDOM_DETAIL_RANDOM_DEVICE_WINCRYPT
#define BOOST_RANDOM_DETAIL_RANDOM_DEVICE_WINCRYPT

#include <boost/config.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if defined(BOOST_WINDOWS)
#include <boost/detail/winapi/config.hpp>

#if BOOST_WINAPI_PARTITION_DESKTOP || BOOST_WINAPI_PARTITION_SYSTEM

#include <boost/core/ignore_unused.hpp>
#include <boost/winapi/crypt.hpp>
#include <boost/winapi/detail/cast_ptr.hpp>
#include <boost/winapi/get_last_error.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/system_error.hpp>
#include <boost/throw_exception.hpp>
#include <string>

#if !defined(BOOST_RANDOM_DEVICE_NO_LIB)
#   if defined(_WIN32_WCE)
#      define BOOST_LIB_NAME "coredll"
#   else
#      define BOOST_LIB_NAME "advapi32"
#   endif
#   define BOOST_AUTO_LINK_NOMANGLE
#   include <boost/config/auto_link.hpp>
#endif

namespace boost {
namespace random {
namespace detail {

template<class Entropy>
class random_device_wincrypt : private noncopyable
{
public:
    typedef Entropy result_type;

    random_device_wincrypt(const std::string& token = std::string())
      : hProv_(NULL)
    {
#if defined(BOOST_NO_ANSI_APIS)
        // Without ANSI APIs, the token is ignored
        ignore_unused(token);
        if (!boost::winapi::CryptAcquireContextW(
            &hProv_,
            NULL,
            NULL,
            boost::winapi::PROV_RNG_,
            boost::winapi::CRYPT_VERIFYCONTEXT_ | boost::winapi::CRYPT_SILENT_))
        {
            BOOST_THROW_EXCEPTION(system::system_error(
                boost::winapi::GetLastError(), system::system_category(), "CryptAcquireContextW"));
        }
#else
        // With ANSI APIs, the behavior is backwards compatible to previous releases
        boost::winapi::CHAR_ buffer[256];
        boost::winapi::DWORD_ type;
        boost::winapi::DWORD_ len;
        std::string provider = token.empty() ? "Microsoft Base Cryptographic Provider v1.0" : token;

        // Find the type of a specific provider
        for (boost::winapi::DWORD_ i = 0; ; ++i)
        {
            len = sizeof(buffer);
            if (!boost::winapi::CryptEnumProvidersA(i, NULL, 0, &type, buffer, &len))
            {
                BOOST_THROW_EXCEPTION(system::system_error(
                    boost::winapi::GetLastError(), system::system_category(), "CryptEnumProvidersA"));
            }
            if (buffer == provider) {
                break;
            }
        }

        if (!boost::winapi::CryptAcquireContextA(
            &hProv_,
            NULL,
            provider.c_str(),
            type,
            boost::winapi::CRYPT_VERIFYCONTEXT_ | boost::winapi::CRYPT_SILENT_))
        {
            BOOST_THROW_EXCEPTION(system::system_error(
                boost::winapi::GetLastError(), system::system_category(), "CryptAcquireContextA"));
        }
#endif
    }

    ~random_device_wincrypt() BOOST_NOEXCEPT
    {
        if (hProv_)
        {
            ignore_unused(boost::winapi::CryptReleaseContext(hProv_, 0));
        }
    }

    result_type operator()()
    {
        result_type result;
        if (!boost::winapi::CryptGenRandom(hProv_, sizeof(result), boost::winapi::detail::cast_ptr(&result)))
        {
            BOOST_THROW_EXCEPTION(system::system_error(
                boost::winapi::GetLastError(), system::system_category(), "CryptGenRandom"));
        }
        return result;
    }

private:
    boost::winapi::HCRYPTPROV_ hProv_;
};

} // detail
} // random
} // boost

#endif // BOOST_WINAPI_PARTITION_DESKTOP || BOOST_WINAPI_PARTITION_SYSTEM
#endif // BOOST_WINDOWS
#endif // BOOST_RANDOM_DETAIL_RANDOM_DEVICE_WINCRYPT
