//
// Copyright (c) 2017 James E. King III
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//   http://www.boost.org/LICENCE_1_0.txt)
//
// BCrypt provider for entropy
//

#include <boost/core/ignore_unused.hpp>
#include <boost/random/entropy_error.hpp>
#include <boost/throw_exception.hpp>
#include <boost/winapi/bcrypt.hpp>
#include <boost/winapi/get_last_error.hpp>

#if defined(BOOST_RANDOM_FORCE_AUTO_LINK) || (!defined(BOOST_ALL_NO_LIB) && !defined(BOOST_RANDOM_NO_LIB))
#   define BOOST_LIB_NAME "bcrypt"
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
        boost::winapi::NTSTATUS_ status =
            boost::winapi::BCryptOpenAlgorithmProvider(
                &hProv_, 
                boost::winapi::BCRYPT_RNG_ALGORITHM_,
                NULL,
                0);

        if (status)
        {
            BOOST_THROW_EXCEPTION(entropy_error(status, "BCryptOpenAlgorithmProvider"));
        }
    }

    ~random_provider() BOOST_NOEXCEPT
    {
        if (hProv_)
        {
            ignore_unused(boost::winapi::BCryptCloseAlgorithmProvider(hProv_, 0));
        }
    }

    //! Obtain entropy and place it into a memory location
    //! \param[in]  buf  the location to write entropy
    //! \param[in]  siz  the number of bytes to acquire
    void get_random_bytes(void *buf, size_t siz)
    {
        boost::winapi::NTSTATUS_ status =
            boost::winapi::BCryptGenRandom(
                hProv_,
                static_cast<boost::winapi::PUCHAR_>(buf),
                siz,
                0);

        if (status)
        {
            BOOST_THROW_EXCEPTION(entropy_error(status, "BCryptGenRandom"));
        }
    }

  private:
    boost::winapi::BCRYPT_ALG_HANDLE_ hProv_;
};

} // detail
} // random
} // boost
