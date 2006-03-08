/* 
 * Copyright Matthias Troyer 2006
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef BOOST_RANDOM_BUFFERED_UNIFORM_01_HPP
#define BOOST_RANDOM_BUFFERED_UNIFORM_01_HPP

#include <boost/random/buffered_generator.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_real.hpp>

#include <iostream>

namespace boost {

/// the abstract base class of a runtime-polymorphic buffered random number generator 
/// generating double values in the interval [0,1[
///
/// \param RealType the floating point type of the random numbers, defaults to double

template <class RealType=double> 
class buffered_uniform_01 
 : public buffered_generator<RealType>
{
public:

  /// the type of random numbers
  typedef RealType result_type;
  
  /// constructs the generator
  /// \param buffer_size the size of the buffer

  buffered_uniform_01(std::size_t buffer_size=BOOST_BUFFERED_GENERATOR_BUFFER_SIZE) 
   :  buffered_generator<result_type>(buffer_size)
  {}

  result_type min BOOST_PREVENT_MACRO_SUBSTITUTION () const { return static_cast<RealType>(0.); }
  result_type max BOOST_PREVENT_MACRO_SUBSTITUTION () const { return static_cast<RealType>(1.); }
};


/// a runtime-polymorphic buffered random number generator 
/// generating double values in the interval [0,1[
///
/// \param Engine the type of random number generator engine
/// \param RealType the floating point type of the random numbers, defaults to double

template <class Engine, class RealType=double> 
class basic_buffered_uniform_01 
 : public buffered_uniform_01<RealType>
{
public:

  /// the type of random numbers
  typedef RealType result_type;
  
  /// the type of random number generator engine
  typedef Engine engine_type;
  
  typedef uniform_real<RealType> distribution_type;
  
  typedef variate_generator<engine_type,distribution_type> generator_type;

  
  /// constructs a default-seeded generator
  /// \param buffer_size the size of the buffer

  basic_buffered_uniform_01(std::size_t buffer_size=BOOST_BUFFERED_GENERATOR_BUFFER_SIZE) 
   : buffered_uniform_01<RealType>(buffer_size)
   , generator_(generator_type(engine_type(),distribution_type()))
  {}

  /// constructs a generator from the given engine
  /// \param engine the engine used to generate values
  /// \param buffer_size the size of the buffer
  ///
  /// If a reference type is specifed as \c Engine type, a reference to the
  /// \c engine is stored and used, otherweise the engine is copied.
  basic_buffered_uniform_01(engine_type engine, std::size_t buffer_size=BOOST_BUFFERED_GENERATOR_BUFFER_SIZE) 
   : buffered_uniform_01<RealType>(buffer_size)
   , generator_(generator_type(engine,distribution_type()))
  {}

private:
  typedef typename buffered_generator<result_type>::buffer_type buffer_type;
  
  /// fills the buffer using the generator
  void fill_buffer(buffer_type& buffer)
  {
    for (typename buffer_type::iterator it=buffer.begin();it!=buffer.end();++it)
      *it=generator_();
  }

  generator_type generator_;
};


} // end namespace boost

#endif // BOOST_RANDOM_BUFFERED_UNIFORM_01_HPP
