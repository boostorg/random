/* boost random/random_device.hpp header file
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
 *  2000-02-18  Portability fixes (thanks to Beman Dawes)
 *  2017-09-04  Made header-only, configurable entropy size, UWP compatible
 */

//  See http://www.boost.org/libs/random for documentation.

#ifndef BOOST_RANDOM_RANDOM_DEVICE_HPP
#define BOOST_RANDOM_RANDOM_DEVICE_HPP

#include <boost/config.hpp>
#include <boost/limits.hpp>
#include <string>

#include <boost/random/detail/random_device_bcrypt.hpp>
#include <boost/random/detail/random_device_file.hpp>
#include <boost/random/detail/random_device_wincrypt.hpp>

namespace boost {
namespace random {

/**
 * Class \random_device models a \nondeterministic_random_number_generator.
 * It uses one or more implementation-defined stochastic processes to
 * generate a sequence of uniformly distributed non-deterministic random
 * numbers. For those environments where a non-deterministic random number
 * generator is not available, class random_device must not be implemented. See
 *
 *  @blockquote
 *  "Randomness Recommendations for Security", D. Eastlake, S. Crocker,
 *  J. Schiller, Network Working Group, RFC 1750, December 1994
 *  @endblockquote
 *
 * for further discussions. 
 *
 * @xmlnote
 * Some operating systems abstract the computer hardware enough
 * to make it difficult to non-intrusively monitor stochastic processes.
 * However, several do provide a special device for exactly this purpose.
 * It seems to be impossible to emulate the functionality using Standard
 * C++ only, so users should be aware that this class may not be available
 * on all platforms.
 * @endxmlnote
 *
 * <b>Implementation Note for Unix</b>
 *
 * On Unix operating systems, the token is interpreted as a filesystem
 * path. It is assumed that this path denotes an operating system
 * pseudo-device which generates a stream of non-deterministic random
 * numbers. The pseudo-device should never signal an error or end-of-file.
 * 
 * By default, \random_device uses the /dev/urandom pseudo-device to retrieve
 * the random numbers. Another option would be to specify the /dev/random
 * pseudo-device, which blocks on reads if the entropy pool has no more
 * random bits available.
 *
 * <b>Implementation Note for Windows</b>
 * 
 * When targeting Windows Desktop or System applications with UWP, or when
 * not using UWP (older SDKs), the traditional wincrypt provider is used to
 * get entropy for backwards compatibility with the previous implementation.
 * This ensures anyone who has developed and use their own crypto provider
 * for random number generation can still use it.  The optional token can 
 * be the name of a wincrypt provider.  If no token is specified, the default 
 * MS_DEF_PROV_A is used.  If BOOST_NO_ANSI_APIS is defined, the token is ignored.
 *
 * For all other windows targets, for example Windows Store UWP targets, 
 * bcrypt is used to acquire entropy.  In these cases, the token is ignored.
 * There are some platform/SDK combinations where it is not possible to have
 * a random device on Windows - for example Windows SDK 8.x with a non-desktop
 * target, since bcrypt is specified as desktop-only in SDK 8.x and expanded to
 * other partitions in SDK 10.x.
 *
 * <b>Compile-Time Definitions</b>
 *
 * [Windows] To force use of bcrypt over wincrypt on desktop platforms,
 * define BOOST_RANDOM_DEVICE_FORCE_BCRYPT
 *
 * [Windows] To disable automatic link libraries being added by providers,
 * define BOOST_RANDOM_DEVICE_NO_LIB
 *
 * [All] To prevent the automatic definition of boost::random::random_device,
 * define BOOST_RANDOM_DEVICE_NO_DEFAULT_IMPL
 *
 * <b>Exceptions</b>
 *
 * Errors will result in a boost::system::system_error exception.
 *
 * <b>Concepts</b>
 *
 * A RandomDeviceProvider must:
 *   - provide a result_type type definition
 *   - provide an operator() implementation returning a result_type
 *
 * A UniformRandomNumberGenerator must (as documented):
 *   - provide a min()
 *   - provide a max()
 *   - provide an operator()
 *
 * The common parts of a UniformRandomNumberGenerator are provided by
 * basic_random_device.
 */
template<class RandomDeviceProvider>
class basic_random_device : public RandomDeviceProvider
{
public:
    basic_random_device(const std::string& token = std::string()) 
        : RandomDeviceProvider(token)
    {
    }

    typedef typename RandomDeviceProvider::result_type result_type;
    BOOST_STATIC_CONSTANT(bool, has_fixed_range = false);

    /** Returns the smallest value that the \random_device can produce. */
    static result_type min BOOST_PREVENT_MACRO_SUBSTITUTION () 
    { 
        return (std::numeric_limits<result_type>::min)(); 
    }

    /** Returns the largest value that the \random_device can produce. */
    static result_type max BOOST_PREVENT_MACRO_SUBSTITUTION() 
    { 
        return (std::numeric_limits<result_type>::max)();
    }

    /**
     * Fills a range with random values.
     * Throws boost::system::system_error if an error occurs.
     */
    template<class Iter>
    void generate(Iter begin, Iter end)
    {
        for(; begin != end; ++begin) 
        {
            *begin = (*this)();
        }
    }
};

//
// Automatically select a default platform specific implementation
//

#if !defined(BOOST_RANDOM_DEVICE_NO_DEFAULT_IMPL)
#if defined(BOOST_WINDOWS)
#if (BOOST_WINAPI_PARTITION_DESKTOP || BOOST_WINAPI_PARTITION_SYSTEM) && !defined(BOOST_RANDOM_DEVICE_FORCE_BCRYPT)
  typedef basic_random_device<detail::random_device_wincrypt<unsigned int> > random_device;
#else
  typedef basic_random_device<detail::random_device_bcrypt<unsigned int> > random_device;
#endif
#else
  typedef basic_random_device<detail::random_device_file<unsigned int> > random_device;
#endif
#endif

} // namespace random

#if !defined(BOOST_RANDOM_DEVICE_NO_DEFAULT_IMPL)
using random::random_device;
#endif

} // namespace boost

#endif /* BOOST_RANDOM_RANDOM_DEVICE_HPP */
