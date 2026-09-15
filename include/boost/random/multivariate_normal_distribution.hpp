/*
 * Copyright Nick Thompson, 2024
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef BOOST_RANDOM_MULTIVARIATE_NORMAL_HPP
#define BOOST_RANDOM_MULTIVARIATE_NORMAL_HPP
#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <random>
#if __has_include(<Eigen/Dense>)
#include <Eigen/Dense>
#else
#error "The Eigen library is required for the operation of this class"
#endif

namespace boost::random {

// This is super useful functionality, but nonetheless it must be shunted off into a dark corner of the library
// because even today there is no standard matrix class and no standard was to do a Cholesky decomposition.
// Hence a more public place in the library just puts users in dependency hell.
template<class RandomAccessContainer>
class multivariate_normal_distribution {
public:
  using Real = typename RandomAccessContainer::value_type;
  multivariate_normal_distribution(RandomAccessContainer const & mean, Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic> const & covariance_matrix) : m_{mean} {
    using std::sqrt;
    if (covariance_matrix.rows() != covariance_matrix.cols()) {
        std::ostringstream oss;
        oss << __FILE__ << ":" << __LINE__ << ":" << __func__;
        oss << ": The covariance matrix must be square, but received a (" << covariance_matrix.rows() << ", " << covariance_matrix.cols() << ") matrix.";
        throw std::domain_error(oss.str());
    }
    if (mean.size() != covariance_matrix.cols()) {
        std::ostringstream oss;
        oss << __FILE__ << ":" << __LINE__ << ":" << __func__;
        oss << ": The mean has dimension " << mean.size() << " but the covariance matrix has " << covariance_matrix.cols() << " columns.";
        throw std::domain_error(oss.str());
    }
    ldlt_.compute(covariance_matrix);
    if(ldlt_.info() != Eigen::Success) {
        std::ostringstream oss;
        oss << __FILE__ << ":" << __LINE__ << ":" << __func__;
        if (ldlt_.info() == Eigen::NumericalIssue) {
          oss << ": The covariance matrix is not positive definite. We probably need to use Eigen::LDLT instead.";
          throw std::domain_error(oss.str());
        } else if (ldlt_.info() == Eigen::InvalidInput) {
          oss << ": Invalid input detected from Eigen.";
          throw std::domain_error(oss.str());
        } else if (ldlt_.info() == Eigen::NoConvergence) {
          oss << ": Iterative procedure did not converge.";
          throw std::domain_error(oss.str());
        }
    }
    if (!ldlt_.isPositive()) {
        std::ostringstream oss;
        oss << __FILE__ << ":" << __LINE__ << ":" << __func__;
        oss << ": The covariance matrix provided is not positive semi-definite.";
        throw std::domain_error(oss.str());
    }
  }

  template<class URNG>
  RandomAccessContainer operator()(URNG& g) const {
    RandomAccessContainer x;
    if constexpr (detail::has_resize_v<RandomAccessContainer>) {
      x.resize(m_.size());
    }
    (*this)(x, g);
    return x;
  }

  template<class URNG>
  void operator()(RandomAccessContainer& x, URNG& g) const {
    using std::normal_distribution;
    if (x.size() != m_.size()) {
      std::ostringstream oss;
      oss << __FILE__ << ":" << __LINE__ << ":" << __func__;
      oss << ": Must provide a vector of the same length as the mean.";
      throw std::domain_error(oss.str());
    }
    
    auto dis = normal_distribution<Real>(0, 1);
    /// First generate standard normal random vector:
    Eigen::Vector<Real, Eigen::Dynamic> u;
    u.resize(x.size());
    for (size_t i = 0; i < x.size(); ++i) {
      u[i] = dis(g);
    }
    // Transform it with the LDLT decomposition:
    // This means: u->PLD^{1/2}u:
    auto const & D = ldlt_.vectorD();
    for (size_t i = 0; i < u.size(); ++i) {
      u[i] = sqrt(D[i])*u[i];
    }
    // Now apply L:
    u = ldlt_.matrixL()*u;
    // And the permutation:
    u = ldlt_.transpositionsP()*u;
    for (size_t i = 0; i < x.size(); ++i) {
      x[i] = u[i] + m_[i];
    }
  }



private:
  RandomAccessContainer m_;
  Eigen::LDLT<Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic> > ldlt_;
};

} // namespace boost::random
#endif
