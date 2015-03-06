/* boost random/vector_io.hpp header file
 *
 * Copyright Steven Watanabe 2011
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org for most recent version including documentation.
 *
 * $Id$
 */

#ifndef BOOST_RANDOM_DETAIL_VECTOR_IO_HPP
#define BOOST_RANDOM_DETAIL_VECTOR_IO_HPP

#include <vector>
#include <iosfwd>
#include <istream>

namespace boost {
namespace random {
namespace detail {

template<class CharT, class Traits, class T>
void print_vector(std::basic_ostream<CharT, Traits>& os,
                  const std::vector<T>& vec)
{
    typename std::vector<T>::const_iterator
        iter = vec.begin(),
        end =  vec.end();
    os << os.widen('[');
    os << os.widen(' ');
    if(iter != end) {
        os << *iter;
        ++iter;
        for(; iter != end; ++iter)
        {
            os << os.widen(' ') << *iter;
        }
    }
    os << os.widen(' ');
    os << os.widen(']');
}

template<class CharT, class Traits, class T>
void read_vector(std::basic_istream<CharT, Traits>& is, std::vector<T>& vec)
{
    CharT ch;
    if(!(is >> ch)) {
        return;
    }
    if(ch != is.widen('[')) {
        is.putback(ch);
        is.setstate(std::ios_base::failbit);
        return;
    }
    T val;
    is >> std::ws;
    while(is >> val >> std::ws) {
        vec.push_back(val);
        if(is.widen(']') == is.peek())
           break;
    }
    if(is.fail())
       return;
    if(!(is >> ch)) {
       return;
    }
    if(ch != is.widen(']')) {
       is.putback(ch);
       is.setstate(std::ios_base::failbit);
    }
}

}
}
}

#endif // BOOST_RANDOM_DETAIL_VECTOR_IO_HPP
