/* boost random/random_device.hpp header file
 *
 * Copyright Jens Maurer 2000
 * Copyright Steven Watanabe 2010-2011
 * Copyright 2017 James E. King III
 * 
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * $Id$
 *
 * Revision history
 *  2000-02-18  Portability fixes (thanks to Beman Dawes)
 *  2017-12-05  Converted to use random_provider
 */

//  See http://www.boost.org/libs/random for documentation.


#ifndef BOOST_RANDOM_RANDOM_DEVICE_HPP
#define BOOST_RANDOM_RANDOM_DEVICE_HPP

#include <boost/core/noncopyable.hpp>
#include <boost/cstdint.hpp>
#include <boost/limits.hpp>
#include <boost/random/detail/random_provider_detect_platform.hpp>
#include <boost/random/detail/random_provider_include_platform.hpp>
#include <boost/random/entropy_error.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_unsigned.hpp>
#include <iterator>

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
 * \random_device offers two methods for entropy generation:
 *
 * 1. Call operator() to generate a random unsigned integer
 * 2. Call get_random_bytes(void *, size_t) to fill any memory
 *    area with random data.
 *
 * The selection logic for the entropy provider is as follows:
 *
 * 1. On CloudABI, or on OpenBSD version 2.1 or later, `arc4random` will be used.
 * 2. On Windows platforms, the `bcrypt` provider is used unless targeting Windows CE or Windows XP, where the `wincrypt` provider is used.
 * 3. On Linux platforms with glibc >= 2.25, `getentropy` is used, otherwise it is treated as a POSIX platform.
 * 4. On POSIX platforms, entropy is obtained by reading from `/dev/urandom`.
 *
 * The following macros can be used:
 *
 * * `BOOST_RANDOM_NO_LIB` (Windows) - disable auto-linking for the `bcrypt` and `wincrypt` provider when building with MSVC
 * * `BOOST_RANDOM_PROVIDER_SHOW` - display the chosen entropy provider at compile time
 */
class random_device : private noncopyable, public detail::random_provider
{
public:
    typedef unsigned int result_type;

    //! \note    this is an old Boost.Random concept requirement
    BOOST_STATIC_CONSTANT(bool, has_fixed_range = false);

    //! \returns the smallest value that the \random_device can produce.
    static result_type min BOOST_PREVENT_MACRO_SUBSTITUTION () 
    {
        return std::numeric_limits<result_type>::min();
    }

    //! \returns the largest value that the random_device can produce.
    static result_type max BOOST_PREVENT_MACRO_SUBSTITUTION ()
    {
        return std::numeric_limits<result_type>::max();
    }

    //! \returns a random value in the range [min(), max()].
    //! \throws  entropy_error
    result_type operator()()
    {
        result_type e;
        get_random_bytes(&e, sizeof(result_type));
        return e;
    }

    //! \returns An entropy estimate for the random numbers returned by
    //!          operator(), in the range min() to log2(max()+1). A
    //!          deterministic random number generator (e.g. a pseudo-random
    //!          number engine) has entropy 0.
    double entropy() const 
    {
        return 10;
    }
    
    //! Allow \random_device to fulfill the SeedSeq concept
    //! and therefore be compatible as a \pseudo_random_number_generator
    //! seed() argument.
    //! 
    //! Example:
    //!    boost::random_device rng;
    //!    boost::mt19937 twister;
    //!    twister.seed(rng);
    //!
    //! \param[in]  first  the first of the sequence requested
    //! \param[in]  last   the last of the sequence requested
    //! \throws  entropy_error
    template<class Iter>
    void generate(Iter first, Iter last)
    {
        typedef typename std::iterator_traits<Iter>::value_type value_type;
        BOOST_STATIC_ASSERT(is_integral<value_type>::value);
        BOOST_STATIC_ASSERT(is_unsigned<value_type>::value);
        BOOST_STATIC_ASSERT(sizeof(value_type) * CHAR_BIT >= 32);

        for (; first != last; ++first)
        {
            get_random_bytes(&*first, sizeof(*first));
            *first &= (std::numeric_limits<boost::uint32_t>::max)();
        }
    }

    //! \returns the name of the selected entropy provider
    const char * name() const
    {
        return BOOST_RANDOM_PROVIDER_STRINGIFY(BOOST_RANDOM_PROVIDER_NAME);
    }
};

} // namespace random

using random::random_device;

} // namespace boost

#endif /* BOOST_RANDOM_RANDOM_DEVICE_HPP */
