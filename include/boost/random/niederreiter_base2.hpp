/* boost random/nierderreiter_base2.hpp header file
 *
 * Copyright Justinas Vygintas Daugmaudis 2010-2018
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_RANDOM_NIEDERREITER_BASE2_HPP
#define BOOST_RANDOM_NIEDERREITER_BASE2_HPP

#include <boost/random/detail/niederreiter_base2_data.hpp>
#include <boost/random/detail/gray_coded_qrng_base.hpp>
#include <boost/random/detail/config.hpp>
#include <boost/random/detail/operators.hpp>

#include <limits>
#include <boost/cstdint.hpp>

#include <boost/dynamic_bitset.hpp>
#include <boost/multi_array.hpp>

//!\file
//!Describes the quasi-random number generator class template niederreiter_base2_engine.
//!
//!\b Note: it is especially useful in conjunction with class template uniform_real.

namespace boost {
namespace random {

/** @cond */
namespace detail {
namespace nb2 {

// Return the base 2 logarithm for a given bitset v
template <typename Block, typename Allocator>
inline typename boost::dynamic_bitset<Block, Allocator>::size_type
bitset_log2(const boost::dynamic_bitset<Block, Allocator>& v)
{
  typedef boost::dynamic_bitset<Block, Allocator> bitset_t;
  typedef typename bitset_t::size_type size_type;

  if (v.none())
    throw std::invalid_argument("bitset_log2");

  size_type up  = v.size() - 1;
  size_type low = v.find_next(0);

  // Binary lookup for the most significant set bit
  while (low < up)
  {
    size_type m = low + (up - low) / 2;

    // Check if any bit is present after mid
    size_type p = v.find_next(m);
    if (p != bitset_t::npos)
        low = p;
    else
        up = m;
  }

  return low;
}


// Multiply polynomials over Z_2.
template <typename Block, typename Allocator>
inline boost::dynamic_bitset<Block, Allocator>
modulo2_multiply(unsigned P, boost::dynamic_bitset<Block, Allocator> v)
{
  boost::dynamic_bitset<Block, Allocator> pt (v.size());
  for (; P; P >>= 1, v <<= 1)
    if (P & 1) pt ^= v;
  return pt;
}


// Calculate the values of the constants V(J,R) as
// described in BFN section 3.3.
//
// px = appropriate irreducible polynomial for current dimension
// pb = polynomial defined in section 2.3 of BFN.
// pb is modified
template <typename Block, typename Allocator, typename T>
inline void calculate_v(const boost::dynamic_bitset<Block, Allocator>& pb,
  typename boost::dynamic_bitset<Block, Allocator>::size_type& pb_degree,
  std::vector<T>& v)
{
  typedef typename boost::dynamic_bitset<Block, Allocator>::size_type size_type;

  const T arbitrary_element = static_cast<T>(1);  // arbitray element of Z_2

  // Now choose a value of Kj as defined in section 3.3.
  // We must have 0 <= Kj < E*J = M.
  // The limit condition on Kj does not seem very relevant
  // in this program.
  size_type kj = pb_degree;

  pb_degree = bitset_log2(pb);

  // Now choose values of V in accordance with
  // the conditions in section 3.3.
  std::fill(v.begin(), v.begin() + kj, T());

  // Quoting from BFN: "Our program currently sets each K_q
  // equal to eq. This has the effect of setting all unrestricted
  // values of v to 1."
  // Actually, it sets them to the arbitrary chosen value.
  // Whatever.
  for (size_type r = kj; r < pb_degree; ++r)
    v[r] = arbitrary_element;

  // Calculate the remaining V's using the recursion of section 2.3,
  // remembering that the B's have the opposite sign.
  for (size_type r = pb_degree; r < v.size(); ++r)
  {
    T term = T /*zero*/ ();
    boost::dynamic_bitset<Block, Allocator> pb_c = pb;
    for (int k = -pb_degree; k < 0; ++k, pb_c >>= 1)
    {
      if( pb_c.test(0) )
        term ^= v[r + k];
    }
    v[r] = term;
  }
}

} // namespace nb2

template<typename IntType, typename Nb2Table>
struct niederreiter_base2_lattice
{
  typedef IntType value_type;

  BOOST_STATIC_CONSTANT(unsigned, bit_count = std::numeric_limits<value_type>::digits);

  explicit niederreiter_base2_lattice(std::size_t dimension)
  {
    resize(dimension);
  }

  void resize(std::size_t dimension)
  {
    detail::dimension_assert("Niederreiter base 2", dimension, Nb2Table::max_dimension);

    // Initialize the bit array
    bits.resize(boost::extents[bit_count][dimension]);

    // Reserve temporary space for lattice computation
    boost::multi_array<value_type, 2> ci(boost::extents[bit_count][bit_count]);

    std::vector<value_type> v;

    // Compute Niedderreiter base 2 lattice
    for (std::size_t dim = 0; dim != dimension; ++dim)
    {
      const unsigned int poly = Nb2Table::polynomial(dim);
      if (poly > std::numeric_limits<value_type>::max()) {
        boost::throw_exception( std::range_error("niederreiter_base2: polynomial value outside the given value type range") );
      }

      const unsigned degree = multiprecision::msb(poly); // integer log2(poly)
      const unsigned max_degree = degree * ((bit_count / degree) + 1);

      v.resize(degree + max_degree);

      // For each dimension, we need to calculate powers of an
      // appropriate irreducible polynomial, see Niederreiter
      // page 65, just below equation (19).
      // Copy the appropriate irreducible polynomial into PX,
      // and its degree into E.  Set polynomial B = PX ** 0 = 1.
      // M is the degree of B.  Subsequently B will hold higher
      // powers of PX.
      boost::dynamic_bitset<>::size_type pb_degree = 0;
      boost::dynamic_bitset<> pb(max_degree, 1);

      unsigned j = 0;
      while (j != bit_count)
      {
        // Now multiply B by PX so B becomes PX**J.
        // In section 2.3, the values of Bi are defined with a minus sign :
        // don't forget this if you use them later!
        nb2::modulo2_multiply(poly, boost::move(pb)).swap(pb);

        // If U = 0, we need to set B to the next power of PX
        // and recalculate V.
        nb2::calculate_v(pb, pb_degree, v);

        // Niederreiter (page 56, after equation (7), defines two
        // variables Q and U.  We do not need Q explicitly, but we
        // do need U.

        // Advance Niederreiter's state variables.
        for (unsigned u = 0; u != degree && j != bit_count; ++u, ++j)
        {
          // Now C is obtained from V.  Niederreiter
          // obtains A from V (page 65, near the bottom), and then gets
          // C from A (page 56, equation (7)).  However this can be done
          // in one step.  Here CI(J,R) corresponds to
          // Niederreiter's C(I,J,R).
          for (unsigned r = 0; r != bit_count; ++r) {
            ci[r][j] = v[r + u];
          }
        }
      }

      // The array CI now holds the values of C(I,J,R) for this value
      // of I.  We pack them into array CJ so that CJ(I,R) holds all
      // the values of C(I,J,R) for J from 1 to NBITS.
      for (unsigned r = 0; r != bit_count; ++r)
      {
        value_type term = 0;
        for (unsigned j = 0; j != bit_count; ++j)
          term = 2*term + ci[r][j];
        bits[r][dim] = term;
      }
    }
  }

  value_type operator()(std::size_t i, std::size_t j) const
  {
    return bits[i][j];
  }

private:
  boost::multi_array<value_type, 2> bits;
};

} // namespace detail
/** @endcond */

//!class template niederreiter_base2_engine implements a quasi-random number generator as described in
//! \blockquote
//!Bratley, Fox, Niederreiter, ACM Trans. Model. Comp. Sim. 2, 195 (1992).
//! \endblockquote
//!
//!In the following documentation @c X denotes the concrete class of the template
//!niederreiter_base2_engine returning objects of type @c IntType, u and v are the values of @c X.
//!
//!Some member functions may throw exceptions of type std::overflow_error. This
//!happens when the quasi-random domain is exhausted and the generator cannot produce
//!any more values. The length of the low discrepancy sequence is given by
//! \f$L=Dimension \times 2^{w}\f$, where `w` = std::numeric_limits<IntType>::digits.
template<typename IntType, typename Nb2Table>
class niederreiter_base2_engine : public detail::gray_coded_qrng_base<
                                              niederreiter_base2_engine<IntType, Nb2Table>
                                            , detail::niederreiter_base2_lattice<IntType, Nb2Table>
                                            , IntType
                                            >
{
  typedef niederreiter_base2_engine<IntType, Nb2Table> self_t;
  typedef detail::niederreiter_base2_lattice<IntType, Nb2Table> lattice_t;
  typedef detail::gray_coded_qrng_base<self_t, lattice_t, IntType> base_t;

public:
  typedef typename base_t::result_type result_type;

  //!Returns: Tight lower bound on the set of values returned by operator().
  //!
  //!Throws: nothing.
  static BOOST_CONSTEXPR result_type min BOOST_PREVENT_MACRO_SUBSTITUTION ()
  { return 0; }

  //!Returns: Tight upper bound on the set of values returned by operator().
  //!
  //!Throws: nothing.
  static BOOST_CONSTEXPR result_type max BOOST_PREVENT_MACRO_SUBSTITUTION ()
  { return (std::numeric_limits<result_type>::max)(); }

  //!Effects: Constructs the default `s`-dimensional Niederreiter base 2 quasi-random number generator.
  //!
  //!Throws: bad_alloc, invalid_argument, range_error.
  explicit niederreiter_base2_engine(std::size_t s)
    : base_t(s) // initialize lattice here
  {}

#ifdef BOOST_RANDOM_DOXYGEN
  //=========================Doxygen needs this!==============================

  //!Returns: The dimension of of the quasi-random domain.
  //!
  //!Throws: nothing.
  std::size_t dimension() const { return base_t::dimension(); }

  //!Requirements: *this is mutable.
  //!
  //!Effects: Resets the quasi-random number generator state to
  //!the one given by the default construction. Equivalent to u.seed(0).
  //!
  //!\brief Throws: nothing.
  void seed()
  {
    base_t::seed();
  }

  //!Requirements: *this is mutable.
  //!
  //!Effects: Effectively sets the quasi-random number generator state to the `init`-th
  //!vector in the `s`-dimensional quasi-random domain, where `s` == X::dimension().
  //!\code
  //!X u, v;
  //!for(int i = 0; i < N; ++i)
  //!    for( std::size_t j = 0; j < u.dimension(); ++j )
  //!        u();
  //!v.seed(N);
  //!assert(u() == v());
  //!\endcode
  //!
  //!\brief Throws: overflow_error.
  void seed(IntType init)
  {
    base_t::seed(init);
  }

  //!Requirements: *this is mutable.
  //!
  //!Returns: Returns a successive element of an `s`-dimensional
  //!(s = X::dimension()) vector at each invocation. When all elements are
  //!exhausted, X::operator() begins anew with the starting element of a
  //!subsequent `s`-dimensional vector.
  //!
  //!Throws: overflow_error.
  result_type operator()()
  {
    return base_t::operator()();
  }

  //!Requirements: *this is mutable.
  //!
  //!Effects: Advances *this state as if `z` consecutive
  //!X::operator() invocations were executed.
  //!\code
  //!X u = v;
  //!for(int i = 0; i < N; ++i)
  //!    u();
  //!v.discard(N);
  //!assert(u() == v());
  //!\endcode
  //!
  //!Throws: overflow_error.
  void discard(IntType z)
  {
    base_t::discard(z);
  }
#endif // BOOST_RANDOM_DOXYGEN
};


/**
 * @attention This specialization of \niederreiter_base2_engine supports up to 4720 dimensions.
 *
 * Binary irreducible polynomials (primes in the ring \f$GF(2)[X]\f$, evaluated at \f$X=2\f$) were generated
 * while they satisfied \f$max(polynomials) < 2^{16}\f$.
 *
 * There are exactly 4720 such primes, which yields a Niederreiter base 2 table for 4720 dimensions.
 *
 * However, it is possible to provide your own table \niederreiter_base2_engine should the default one be insufficient.
 */
typedef niederreiter_base2_engine<boost::uint_least64_t,
  #ifdef BOOST_RANDOM_DOXYGEN
    niedereiter-base2-table
  #else
    detail::qrng_tables::niederreiter_base2
  #endif
> niederreiter_base2;

} // namespace random

} // namespace boost

#endif // BOOST_RANDOM_NIEDERREITER_BASE2_HPP
