/* boost random/detail/seed.hpp header file
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

#ifndef BOOST_RANDOM_DETAIL_TRAITS_HPP
#define BOOST_RANDOM_DETAIL_TRAITS_HPP

#include <boost/type_traits/is_signed.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/make_unsigned.hpp>
#include <boost/mpl/bool.hpp>
#include <limits>

namespace boost {
namespace random {
namespace traits {

   template <class T, bool intrinsic>
   struct make_unsigned_imp
   {
      typedef typename boost::make_unsigned<T>::type type;
   };
   template <class T>
   struct make_unsigned_imp<T, false>
   {
      BOOST_STATIC_ASSERT(std::numeric_limits<T>::is_specialized);
      BOOST_STATIC_ASSERT(std::numeric_limits<T>::is_signed == false);
      BOOST_STATIC_ASSERT(std::numeric_limits<T>::is_integer == true);
      typedef T type;
   };

   template <class T>
   struct make_unsigned : public make_unsigned_imp < T, boost::is_integral<T>::value > {};

   template <class T, bool intrinsic>
   struct make_unsigned_or_unbounded_imp
   {
      typedef typename boost::make_unsigned<T>::type type;
   };
   template <class T>
   struct make_unsigned_or_unbounded_imp<T, false>
   {
      BOOST_STATIC_ASSERT(std::numeric_limits<T>::is_specialized);
      BOOST_STATIC_ASSERT((std::numeric_limits<T>::is_signed == false) || (std::numeric_limits<T>::is_bounded == false));
      BOOST_STATIC_ASSERT(std::numeric_limits<T>::is_integer == true);
      typedef T type;
   };

   template <class T>
   struct make_unsigned_or_unbounded : public make_unsigned_or_unbounded_imp<T, boost::is_integral<T>::value> {};


   template <class T>
   struct is_integral
      : public mpl::bool_<boost::is_integral<T>::value || (std::numeric_limits<T>::is_integer)>
   {
   };

   template <class T> struct is_signed
      : public mpl::bool_ < boost::is_signed<T>::value || (std::numeric_limits<T>::is_specialized && std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_signed)>
   {};



}
}
}

#include <boost/random/detail/enable_warnings.hpp>

#endif
