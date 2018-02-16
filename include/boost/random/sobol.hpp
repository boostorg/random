/* boost random/sobol.hpp header file
 *
 * Copyright Justinas Vygintas Daugmaudis 2010-2018
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_RANDOM_SOBOL_HPP
#define BOOST_RANDOM_SOBOL_HPP

#include <boost/random/detail/sobol_data.hpp>
#include <boost/random/detail/gray_coded_qrng_base.hpp>

#include <limits>

#include <boost/cstdint.hpp>

#include <boost/multi_array.hpp>

//!\file
//!Describes the quasi-random number generator class template sobol_engine.
//!
//!\b Note: it is especially useful in conjunction with class template uniform_real.

namespace boost {
namespace random {

/** @cond */
namespace detail {

// sobol_lattice sets up the random-number generator to produce a Sobol
// sequence of at most max dims-dimensional quasi-random vectors.
// Adapted from ACM TOMS algorithm 659, see

// http://doi.acm.org/10.1145/42288.214372

template<typename IntType, typename SobolTables>
struct sobol_lattice
{
  typedef IntType value_type;

  BOOST_STATIC_CONSTANT(unsigned, bit_count = std::numeric_limits<IntType>::digits);

  // default copy c-tor is fine

  explicit sobol_lattice(std::size_t dimension)
  {
    resize(dimension);
  }

  void resize(std::size_t dimension)
  {
    detail::dimension_assert("Sobol", dimension, SobolTables::max_dimension);

    // Initialize the bit array
    bits.resize(boost::extents[bit_count][dimension]);

    // Initialize direction table in dimension 0
    for (unsigned k = 0; k != bit_count; ++k)
      bits[k][0] = static_cast<value_type>(1);

    // Initialize in remaining dimensions.
    for (std::size_t dim = 1; dim < dimension; ++dim)
    {
      const unsigned int poly = SobolTables::polynomial(dim-1);
      if (poly > std::numeric_limits<value_type>::max()) {
        boost::throw_exception( std::range_error("sobol: polynomial value outside the given value type range") );
      }
      const unsigned degree = multiprecision::detail::find_msb(poly); // integer log2(poly)

      // set initial values of m from table
      for (unsigned k = 0; k != degree; ++k)
        bits[k][dim] = SobolTables::minit(k, dim-1);

      // Calculate remaining elements for this dimension,
      // as explained in Bratley+Fox, section 2.
      for (unsigned j = degree; j < bit_count; ++j)
      {
        unsigned int p_i = poly;
        bits[j][dim] = bits[j - degree][dim];
        for (unsigned k = 0; k != degree; ++k)
        {
          int rem = degree - k;
          bits[j][dim] ^= ((p_i & 1) * bits[j-rem][dim]) << rem;
          p_i >>= 1;
        }
      }
    }

    // Shift columns by appropriate power of 2.
    value_type p = static_cast<value_type>(1);
    for (int j = bit_count-1-1; j >= 0; --j, ++p)
      for (std::size_t dim = 0; dim != dimension; ++dim)
        bits[j][dim] <<= p;
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

//!class template sobol_engine implements a quasi-random number generator as described in
//! \blockquote
//![Bratley+Fox, TOMS 14, 88 (1988)]
//!and [Antonov+Saleev, USSR Comput. Maths. Math. Phys. 19, 252 (1980)]
//! \endblockquote
//!
//!In the following documentation @c X denotes the concrete class of the template
//!sobol_engine returning objects of type @c IntType, u and v are the values of @c X.
//!
//!Some member functions may throw exceptions of type @c std::overflow_error. This
//!happens when the quasi-random domain is exhausted and the generator cannot produce
//!any more values. The length of the low discrepancy sequence is given by \f$L=Dimension \times 2^{w}\f$,
//!where `w`= std::numeric_limits<IntType>::digits.
template<typename IntType, typename SobolTables>
class sobol_engine : public detail::gray_coded_qrng_base<
                                sobol_engine<IntType, SobolTables>
                              , detail::sobol_lattice<IntType, SobolTables>
                              , IntType
                              >
{
  typedef sobol_engine<IntType, SobolTables> self_t;
  typedef detail::sobol_lattice<IntType, SobolTables> lattice_t;
  typedef detail::gray_coded_qrng_base<self_t, lattice_t, IntType> base_t;

public:
  typedef typename base_t::result_type result_type;

  /** @copydoc boost::random::niederreiter_base2_engine::min() */
  static BOOST_CONSTEXPR result_type min BOOST_PREVENT_MACRO_SUBSTITUTION ()
  { return 0; }

  /** @copydoc boost::random::niederreiter_base2_engine::max() */
  static BOOST_CONSTEXPR result_type max BOOST_PREVENT_MACRO_SUBSTITUTION ()
  { return (std::numeric_limits<result_type>::max)(); }

  //!Effects: Constructs the default `s`-dimensional Sobol quasi-random number generator.
  //!
  //!Throws: bad_alloc, invalid_argument, range_error.
  explicit sobol_engine(std::size_t s)
    : base_t(s)
  {}

  // default copy c-tor is fine

#ifdef BOOST_RANDOM_DOXYGEN
  //=========================Doxygen needs this!==============================

  /** @copydoc boost::random::niederreiter_base2_engine::dimension() */
  std::size_t dimension() const { return base_t::dimension(); }

  /** @copydoc boost::random::niederreiter_base2_engine::seed() */
  void seed()
  {
    base_t::seed();
  }

  /** @copydoc boost::random::niederreiter_base2_engine::seed(IntType) */
  void seed(IntType init)
  {
    base_t::seed(init);
  }

  /** @copydoc boost::random::niederreiter_base2_engine::operator()() */
  result_type operator()()
  {
    return base_t::operator()();
  }

  /** @copydoc boost::random::niederreiter_base2_engine::discard(IntType) */
  void discard(IntType z)
  {
    base_t::discard(z);
  }
#endif // BOOST_RANDOM_DOXYGEN
};

/**
 * @attention This specialization of \sobol_engine supports up to 3667 dimensions.
 *
 * Data on the primitive binary polynomials \f$a\f$ and the corresponding starting values \f$m\f$,
 * for Sobol sequences in up to 21201 dimensions, taken from
 *
 *  @blockquote
 *  S. Joe and F. Y. Kuo, Constructing Sobol sequences with better two-dimensional projections,
 *  SIAM J. Sci. Comput. 30, 2635-2654 (2008).
 *  @endblockquote
 *
 * For practical reasons the default table uses only the subset of binary polynomials \f$a\f$ that
 * satisfy \f$\forall a < 2^{16}\f$.
 *
 * However, it is possible to provide your own table should the default one be insufficient.
 */
typedef sobol_engine<boost::uint_least64_t,
  #ifdef BOOST_RANDOM_DOXYGEN
    sobol-tables
  #else
    detail::qrng_tables::sobol
  #endif
> sobol;

} // namespace random

} // namespace boost

#endif // BOOST_RANDOM_SOBOL_HPP
