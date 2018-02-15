/* boost random/detail/gray_coded_qrng_base.hpp header file
 *
 * Copyright Justinas Vygintas Daugmaudis 2010-2018
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_RANDOM_DETAIL_GRAY_CODED_QRNG_BASE_HPP
#define BOOST_RANDOM_DETAIL_GRAY_CODED_QRNG_BASE_HPP

#include <boost/random/detail/qrng_base.hpp>

// Prerequisite headers for bitscan to work
#include <limits.h>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/make_unsigned.hpp>
#include <boost/multiprecision/detail/bitscan.hpp> // find_lsb

//!\file
//!Describes the gray-coded quasi-random number generator base class template.

namespace boost {
namespace random {

namespace detail {

template<typename IntType, typename DerivedT, typename LatticeT>
class gray_coded_qrng_base : public qrng_base<IntType, DerivedT, LatticeT>
{
private:
  typedef gray_coded_qrng_base<IntType, DerivedT, LatticeT> self_t;
  typedef qrng_base<IntType, DerivedT, LatticeT> base_t;

  // The base needs to access modifying member f-ns, and we
  // don't want these functions to be available for the public use
  friend class qrng_base<IntType, DerivedT, LatticeT>;

public:
  typedef typename base_t::size_type size_type;
  typedef typename LatticeT::value_type result_type;

  explicit gray_coded_qrng_base(std::size_t dimension)
    : base_t(dimension)
  {}

  // default copy c-tor is fine

  // default assignment operator is fine

  void seed()
  {
    base_t::set_zero();
    update_quasi(0);
  }

  void seed(size_type init)
  {
    this->curr_elem = 0;
    if (init != this->seq_count)
    {
      base_t::set_zero();

      this->seq_count = init++;
      init ^= (init >> 1);
      for (unsigned r = 0; init != 0; ++r, init >>= 1)
      {
        if (init & 1)
          update_quasi(r);
      }
    }
  }

private:
  void compute_seq(size_type cnt)
  {
    // Find the position of the least-significant zero in sequence count.
    // This is the bit that changes in the Gray-code representation as
    // the count is advanced.
    update_quasi(multiprecision::detail::find_lsb(~cnt));
  }

  void update_quasi(unsigned r)
  {
    // Calculate the next state.
    for (std::size_t i = 0; i != this->dimension(); ++i)
      this->quasi_state[i] ^= this->lattice(r, i);
  }
};

}} // namespace detail::random

} // namespace boost

#endif // BOOST_RANDOM_DETAIL_GRAY_CODED_QRNG_BASE_HPP
