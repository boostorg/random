/* boost random/detail/config.hpp header file
 *
 * Copyright Steven Watanabe 2009
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org for most recent version including documentation.
 *
 * $Id$
 */

#include <boost/config.hpp>

#if (defined(BOOST_NO_OPERATORS_IN_NAMESPACE) || defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)) \
    && !defined(BOOST_MSVC)
    #define BOOST_RANDOM_NO_STREAM_OPERATORS
#endif

// WinRT target.
#if !defined(BOOST_RANDOM_WINDOWS_RUNTIME)
# if defined(__cplusplus_winrt)
#  include <winapifamily.h>
#  if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#   define BOOST_RANDOM_WINDOWS_RUNTIME 1
#  endif
# endif
#endif
