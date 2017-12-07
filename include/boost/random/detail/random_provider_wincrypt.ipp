/* boost random/detail/random_provider_wincrypt implementation
*
* Copyright Jens Maurer 2000
* Copyright 2007 Andy Tompkins.
* Copyright Steven Watanabe 2010-2011
* Copyright 2017 James E. King III
*
* Distributed under the Boost Software License, Version 1.0. (See
* accompanying file LICENSE_1_0.txt or copy at
* http://www.boost.org/LICENSE_1_0.txt)
*
* $Id$
*/

#include <boost/core/ignore_unused.hpp>
#include <boost/random/entropy_error.hpp>
#include <boost/throw_exception.hpp>
#include <boost/winapi/crypt.hpp>
#include <boost/winapi/get_last_error.hpp>

#if defined(BOOST_RANDOM_FORCE_AUTO_LINK) || (!defined(BOOST_ALL_NO_LIB) && !defined(BOOST_RANDOM_NO_LIB))
#   if defined(_WIN32_WCE)
#      define BOOST_LIB_NAME "coredll"
#   else
#      define BOOST_LIB_NAME "advapi32"
#   endif
#   define BOOST_AUTO_LINK_NOMANGLE
#   include <boost/config/auto_link.hpp>
#   undef BOOST_AUTO_LINK_NOMANGLE
#endif

namespace boost {
namespace random {
namespace detail {

class random_provider
{
  public:
    random_provider()
        : hProv_(NULL)
    {
        if (!boost::winapi::CryptAcquireContextW(
            &hProv_,
            NULL,
            NULL,
            boost::winapi::PROV_RSA_FULL_,
            boost::winapi::CRYPT_VERIFYCONTEXT_ | boost::winapi::CRYPT_SILENT_))
        {
            boost::winapi::DWORD_ err = boost::winapi::GetLastError();
            BOOST_THROW_EXCEPTION(entropy_error(err, "CryptAcquireContext"));
        }
    }

    ~random_provider() BOOST_NOEXCEPT
    {
        if (hProv_)
        {
            ignore_unused(boost::winapi::CryptReleaseContext(hProv_, 0));
        }
    }

    //! Obtain entropy and place it into a memory location
    //! \param[in]  buf  the location to write entropy
    //! \param[in]  siz  the number of bytes to acquire
    void get_random_bytes(void *buf, size_t siz)
    {
        if (!boost::winapi::CryptGenRandom(hProv_, siz, 
                    static_cast<boost::winapi::BYTE_ *>(buf)))
        {
            boost::winapi::DWORD_ err = boost::winapi::GetLastError();
            BOOST_THROW_EXCEPTION(entropy_error(err, "CryptGenRandom"));
        }
    }

  private:
    boost::winapi::HCRYPTPROV_ hProv_;
};

} // detail
} // random
} // boost
