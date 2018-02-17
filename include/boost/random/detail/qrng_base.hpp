/* boost random/detail/qrng_base.hpp header file
 *
 * Copyright Justinas Vygintas Daugmaudis 2010-2017
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_RANDOM_DETAIL_QRNG_BASE_HPP
#define BOOST_RANDOM_DETAIL_QRNG_BASE_HPP

#include <istream>
#include <ostream>

#include <stdexcept>
#include <vector>
#include <sstream>

#include <boost/random/detail/operators.hpp>

#include <boost/throw_exception.hpp>

//!\file
//!Describes the quasi-random number generator base class template.

namespace boost {
namespace random {

namespace detail {

template<typename DerivedT, typename LatticeT, typename SizeT>
class qrng_base
{
public:
  typedef SizeT size_type;
  typedef typename LatticeT::value_type result_type;

  explicit qrng_base(std::size_t dimension)
    // Guard against invalid dimensions before creating the lattice
    : lattice(prevent_zero_dimension(dimension))
    , quasi_state(dimension)
  {
    derived().seed();
  }

  // default copy c-tor is fine

  // default assignment operator is fine

  //!Returns: The dimension of of the quasi-random domain.
  //!
  //!Throws: nothing.
  std::size_t dimension() const { return quasi_state.size(); }

  //!Requirements: *this is mutable.
  //!
  //!Returns: Returns a successive element of an s-dimensional
  //!(s = X::dimension()) vector at each invocation. When all elements are
  //!exhausted, X::operator() begins anew with the starting element of a
  //!subsequent s-dimensional vector.
  //!
  //!Throws: overflow_error.
  result_type operator()()
  {
    return curr_elem != dimension() ? load_cached(): next_state();
  }

  //!Fills a range with quasi-random values.
  template<typename Iter> void generate(Iter first, Iter last)
  {
    for (; first != last; ++first)
      *first = this->operator()();
  }

  //!Requirements: *this is mutable.
  //!
  //!Effects: Advances *this state as if z consecutive
  //!X::operator() invocations were executed.
  //!
  //!Throws: overflow_error.
  void discard(size_type z)
  {
    const std::size_t dimension_value = dimension();

    size_type vec_n  = z / dimension_value;
    size_type elem_n = z - vec_n * dimension_value; // z % Dimension
    size_type vec_offset = vec_n + (curr_elem + elem_n) / dimension_value;
    // Discards vec_offset consecutive s-dimensional vectors
    discard_vector(vec_offset);
    // Sets up the proper position of the element-to-read
    curr_elem += (z - dimension_value * vec_offset);
  }

  //!Writes a @c DerivedT to a @c std::ostream.
  BOOST_RANDOM_DETAIL_OSTREAM_OPERATOR(os, DerivedT, s)
  {
    os << s.dimension() << " " << s.seq_count << " " << s.curr_elem;
    return os;
  }

  //!Reads a @c DerivedT from a @c std::istream.
  BOOST_RANDOM_DETAIL_ISTREAM_OPERATOR(is, DerivedT, s)
  {
    std::size_t dim;
    typename DerivedT::size_type seed, z;
    if (is >> dim >> std::ws >> seed >> std::ws >> z) // initialize iff success!
    {
      if (s.dimension() != dim)
      {
        prevent_zero_dimension(dim);
        s.lattice.resize(dim);
        s.quasi_state.resize(dim);
      }
      // Fast-forward to the correct state
      s.seed(seed);
      s.discard(z);
    }
    return is;
  }

  //!Returns true if the two generators will produce identical sequences.
  BOOST_RANDOM_DETAIL_EQUALITY_OPERATOR(DerivedT, x, y)
  {
    const std::size_t dimension_value = x.dimension();

    // Note that two generators with different seq_counts and curr_elems can
    // produce the same sequence because the generator triple
    // (D, S, D) is equivalent to (D, S + 1, 0), where D is dimension, S -- seq_count,
    // and the last one is curr_elem.

    return (dimension_value == y.dimension()) &&
      (x.seq_count + (x.curr_elem / dimension_value) == y.seq_count + (y.curr_elem / dimension_value)) &&
      (x.curr_elem % dimension_value == y.curr_elem % dimension_value);
  }

  //!Returns true if the two generators will produce different sequences,
  BOOST_RANDOM_DETAIL_INEQUALITY_OPERATOR(DerivedT)

protected:
  DerivedT& derived() throw()
  {
    return *static_cast<DerivedT * const>(this);
  }

  void set_zero()
  {
    curr_elem = 0;
    seq_count = 0;
    std::fill(quasi_state.begin(), quasi_state.end(), result_type /*zero*/());
  }

private:
  inline static std::size_t prevent_zero_dimension(std::size_t dimension)
  {
    if (dimension == 0)
      boost::throw_exception( std::invalid_argument("qrng_base: zero dimension") );
    return dimension;
  }

  // Load the result from the saved state.
  result_type load_cached()
  {
    return quasi_state[curr_elem++];
  }

  result_type next_state()
  {
    size_type new_seq = seq_count;
    if (++new_seq > seq_count)
    {
      derived().compute_seq(new_seq);
      seq_count = new_seq;
      curr_elem = 0;
      return load_cached();
    }
    boost::throw_exception( std::overflow_error("qrng_base: next_state") );
  }

  // Discards z consecutive s-dimensional vectors,
  // and preserves the position of the element-to-read
  void discard_vector(size_type z)
  {
    size_type inc_seq_count = seq_count + z;
    // Here we check that no overflow occurs before we
    // begin seeding the new value
    if (inc_seq_count > seq_count)
    {
      std::size_t tmp = curr_elem;

      derived().seed(inc_seq_count);

      curr_elem = tmp;
    }
    else if (inc_seq_count < seq_count) // Increment overflowed?
    {
      boost::throw_exception( std::overflow_error("discard_vector") );
    }
  }

protected:
  LatticeT lattice;
  std::size_t curr_elem;
  size_type seq_count;
  std::vector<result_type> quasi_state;
};

inline void dimension_assert(const char* generator, std::size_t dim, std::size_t maxdim)
{
  if (dim > maxdim)
  {
    std::ostringstream os;
    os << "The " << generator << " quasi-random number generator only supports up to "
      << maxdim << " dimensions.";
    boost::throw_exception( std::invalid_argument(os.str()) );
  }
}

}} // namespace detail::random

} // namespace boost

#endif // BOOST_RANDOM_DETAIL_QRNG_BASE_HPP
