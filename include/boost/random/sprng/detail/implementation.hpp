/* 
 * Copyright Matthias Troyer 2006
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
*
 */

#include <iostream>
#include <cassert>
#include <stdexcept>
#include <cstdlib>
#include <boost/throw_exception.hpp>
#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <boost/random/sprng/detail/buffer.hpp>
#include <boost/random/parallel/keyword.hpp>
#include <boost/parameter/macros.hpp>
#include <boost/preprocessor/cat.hpp>

#if !defined(BOOST_SPRNG_GENERATOR)
#error Please set BOOST_SPRNG_GENERATOR to the name of one of the SPRNG generators before including this header file
#elif !defined(BOOST_SPRNG_TYPE)
#error Please set BOOST_SPRNG_TYPE to the type of one of the SPRNG generators before including this header file
#elif !defined(BOOST_SPRNG_MAX_STREAMS)
#error Please set BOOST_SPRNG_MAX_STREAMS to the maximum number of streams supported by the SPRNG generator
#elif !defined(BOOST_SPRNG_MAX_PARAMS)
#error Please set BOOST_SPRNG_MAX_PARAMS to the maximum number of para,meters supported by the SPRNG generator
#else

#define BOOST_SPRNG_CALL(FUN) BOOST_PP_CAT(BOOST_SPRNG_GENERATOR,BOOST_PP_CAT(_,FUN))

#include <boost/random/sprng/detail/interface.hpp>

namespace boost { namespace random { namespace sprng {

/// wrapper for the 64-bit linear congruential generator of the SPRNG library
class BOOST_SPRNG_GENERATOR
{
public:
  /// SPRNG generators are all floating point generators at the moment
  typedef double result_type;
  enum { has_fixed_range = false};
  enum { sprng_type = BOOST_SPRNG_TYPE};
  
  /// the maximum number of independent streams
  enum { max_streams = BOOST_SPRNG_MAX_STREAMS };
  /// the number of different parameters for the constructor and seeding
  enum { max_param = BOOST_SPRNG_MAX_PARAMS };

  
  BOOST_SPRNG_GENERATOR()
   : sprng_ptr(0)
  {
    seed();
  }

  


  // we need custom copy constructor, destructor and assignment operators
  BOOST_SPRNG_GENERATOR(BOOST_SPRNG_GENERATOR const& rhs)
  {
    random::sprng::detail::buffer buf(rhs.sprng_ptr,&BOOST_SPRNG_CALL(pack_rng));
    sprng_ptr = buf.unpack(&BOOST_SPRNG_CALL(unpack_rng));
  }

  ~BOOST_SPRNG_GENERATOR()
  {
    free();
  }

  template<class It>
  BOOST_SPRNG_GENERATOR(It& first, It const& last)
   : sprng_ptr(0)
  {
    seed(first,last);
  }


#define BOOST_RANDOM_LCG64_CONSTRUCTOR(z, n, unused) \
  template <BOOST_PP_ENUM_PARAMS(n,class T)>         \
  BOOST_SPRNG_GENERATOR(BOOST_PP_ENUM_BINARY_PARAMS(n,T,const& x))   \
   : sprng_ptr(0)                                    \
  {                                                  \
    seed(BOOST_PP_ENUM_PARAMS(n,x) );                \
  }
   
BOOST_PP_REPEAT_FROM_TO(1, 5, BOOST_RANDOM_LCG64_CONSTRUCTOR,~)

  BOOST_SPRNG_GENERATOR const& operator=(BOOST_SPRNG_GENERATOR const& rhs)
  {
    free();
    detail::buffer buf(rhs.sprng_ptr,&BOOST_SPRNG_CALL(pack_rng));
    sprng_ptr = buf.unpack(&BOOST_SPRNG_CALL(unpack_rng));
    return *this;
  }

  /// seed function taking the SPRNG intialization arguments
  /// @param stream The number of this streamuence. Needs to be less than the num_stream parameter.
  /// @param num_stream The total number of random number streams to be created. Needs to be less than max_streams
  /// @param s the common seed for all random number streams
  /// @param param optional parametrization of the generator. Needs to be less than max_param.
  ///
  /// The SPRNG library guarantees that if the global_seed and param arguments are the same, 
  /// the random number streams specified by stream (0 <= stream < num_stream) are independent and non-overlapping.
  /// Changing eiher global_seed or the param will lead to different random number streams

  void seed_implementation(unsigned int stream=0, unsigned int num_stream=1, int s=0, unsigned int param=0)
  {
    free();
    BOOST_ASSERT(stream < num_stream);
    BOOST_ASSERT(num_stream < max_streams);
    BOOST_ASSERT(param < max_param);
    sprng_ptr= BOOST_SPRNG_CALL(init_rng) (sprng_type, stream, num_stream, s, param);
    if (sprng_ptr==0)
      boost::throw_exception(std::runtime_error("Failed initializing SPRNG LCG64 generator"));
  }
  
  /// seed with the default.
  void seed()
  {
    seed_implementation();
  }

  // forwarding seed functions
  BOOST_PARAMETER_MEMFUN(void,seed,1,4,parallel_seed_params)
  {
    seed_implementation(p[stream_number|0u], p[total_streams|1u], p[global_seed|0], p[parameter|0u]);
  }
  
  /// seed with a range of unsigned int values
  template<class It>
  void seed(It& first, It const& last)
  {
    if(first == last)
      throw std::invalid_argument("boost::sprng::BOOST_SPRNG_GENERATOR::seed");
    seed(global_seed=*first++);
  }

  result_type min BOOST_PREVENT_MACRO_SUBSTITUTION () const { return 0.; }
  result_type max BOOST_PREVENT_MACRO_SUBSTITUTION () const { return 1.; }

  /// return the next value
  result_type operator()()
  {
    return BOOST_SPRNG_CALL(get_rn_dbl)(sprng_ptr);
  }

  static bool validation(result_type x) 
  {
    return false; // MISSING
  }

#ifdef BOOST_NO_OPERATORS_IN_NAMESPACE
    
  // Use a member function; Streamable concept not supported.
  bool operator==(const BOOST_SPRNG_GENERATOR& rhs) const
  {
    detail::buffer buf1(sprng_ptr,&BOOST_SPRNG_CALL(pack_rng));
    detail::buffer buf2(rhs.sprng_ptr,&BOOST_SPRNG_CALL(pack_rng));
    return buf1 == buf2;
  }
  bool operator!=(const BOOST_SPRNG_GENERATOR& rhs) const
  { return !(*this == rhs); }

#else 
  friend bool operator==(const BOOST_SPRNG_GENERATOR& x,
                         const BOOST_SPRNG_GENERATOR& y)
  { 
    detail::buffer buf1(x.sprng_ptr,&BOOST_SPRNG_CALL(pack_rng));
    detail::buffer buf2(y.sprng_ptr,&BOOST_SPRNG_CALL(pack_rng));
    return buf1 == buf2;
  }
  
  friend bool operator!=(const BOOST_SPRNG_GENERATOR& x,
                         const BOOST_SPRNG_GENERATOR& y)
  { return !(x == y); }
    
#if !defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS) && !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x551))
  template<class CharT, class Traits>
  friend std::basic_ostream<CharT,Traits>&
  operator<<(std::basic_ostream<CharT,Traits>& os,
             const BOOST_SPRNG_GENERATOR& lcg)
  {
    detail::buffer buf(lcg.sprng_ptr,&BOOST_SPRNG_CALL(pack_rng));
    buf.write(os);
    return os;
  }

  template<class CharT, class Traits>
  friend std::basic_istream<CharT,Traits>&
  operator>>(std::basic_istream<CharT,Traits>& is,
             BOOST_SPRNG_GENERATOR& lcg)
  {
    detail::buffer buf;
    buf.read(is);
    lcg.sprng_ptr = buf.unpack(&BOOST_SPRNG_CALL(unpack_rng));
    return is;
  }
 
private:
#endif
#endif

  void free()
  {
    if(sprng_ptr)
      BOOST_SPRNG_CALL(free_rng(sprng_ptr));
    sprng_ptr=0;
  }
  
  int *sprng_ptr;    
};

// probably needs the "no native streams" caveat for STLPort
#if !defined(__SGI_STL_PORT) && BOOST_WORKAROUND(__GNUC__, == 2)
inline std::ostream& operator<<(std::ostream& os, const BOOST_SPRNG_GENERATOR& lcg)
{
    detail::buffer buf(lcg.sprng_ptr,&BOOST_SPRNG_CALL(pack_rng));
    buf.write(os);
    return os;
}

inline std::istream& operator>>(std::istream& is, BOOST_SPRNG_GENERATOR& lcg)
{
    detail::buffer buf;
    buf.read(is);
    lcg.sprng_ptr = buf.unpack(&BOOST_SPRNG_CALL(unpack_rng));
    return is;
}

#elif defined(BOOST_NO_OPERATORS_IN_NAMESPACE) || defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS) || BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x551))
template<class CharT, class Traits>
std::basic_ostream<CharT,Traits>& operator<<(std::basic_ostream<CharT,Traits>& os, const BOOST_SPRNG_GENERATOR& lcg)
{
    detail::buffer buf(lcg.sprng_ptr,&BOOST_SPRNG_CALL(pack_rng));
    buf.write(os);
    return os;
}

template<class CharT, class Traits>
std::basic_istream<CharT,Traits>& operator>>(std::basic_istream<CharT,Traits>& is, BOOST_SPRNG_GENERATOR& lcg)
{
    detail::buffer buf;
    buf.read(is);
    lcg.sprng_ptr = buf.unpack(&BOOST_SPRNG_CALL(unpack_rng));
    return is;
}
#endif

} } } // namespace boost::random::sprng

#endif 

#undef BOOST_SPRNG_CALL
