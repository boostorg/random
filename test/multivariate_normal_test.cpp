/*
 * Copyright Nick Thompson, 2024
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "math_unit_test.hpp"
#include <boost/random/multivariate_normal_distribution.hpp>
#include <array>
#include <algorithm>
#include <cmath>
#include <random>
#include <limits>

using std::abs;
using boost::random::multivariate_normal_distribution;

template <class Real> void test_multivariate_normal() {
  using Eigen::Matrix;
  using Eigen::Dynamic;
  constexpr const size_t n = 7;
  Matrix<Real, Dynamic, Dynamic> C = Matrix<Real, Dynamic, Dynamic>::Identity(n, n);
  std::mt19937_64 mt(12345);
  std::array<Real, n> mean;
  std::uniform_real_distribution<Real> dis(-1, 1);
  std::generate(mean.begin(), mean.end(), [&]() { return dis(mt); });
  auto mvn = multivariate_normal_distribution<decltype(mean)>(mean, C);
  std::array<Real, n> x;
  std::array<Real, n> empirical_means;
  empirical_means.fill(0);

  size_t i = 0;
  size_t samples = 2048;
  do {
    x = mvn(mt);
    for (size_t j = 0; j < n; ++j) {
      empirical_means[j] += x[j];
    }
  } while(i++ < samples);

  for (size_t j = 0; j < n; ++j) {
    empirical_means[j] /= samples;
    CHECK_ABSOLUTE_ERROR(mean[j], empirical_means[j], 0.05);
  }

  // Exhibits why we need to use the LDL^T decomposition:
  C = Matrix<Real, Dynamic, Dynamic>::Zero(n, n);
  mvn = multivariate_normal_distribution<decltype(mean)>(mean, C);
  i = 0;
  do {
    x = mvn(mt);
    for (size_t j = 0; j < n; ++j) {
      CHECK_EQUAL(mean[j], x[j]);
    }
  } while(i++ < 10);
  // Test that we're applying the permutation matrix correctly:
  C = Matrix<Real, Dynamic, Dynamic>::Zero(n, n);
  C(0,0) = 1;
  mvn = multivariate_normal_distribution<decltype(mean)>(mean, C);
  i = 0;
  do {
    x = mvn(mt);
    for (size_t j = 1; j < mean.size(); ++j) {
      CHECK_EQUAL(mean[j], x[j]);
    }
  } while(i++ < 3);

  C(0,0) = 0;
  C(n-1,n-1) = 1;
  mvn = multivariate_normal_distribution<decltype(mean)>(mean, C);
  i = 0;
  do {
    x = mvn(mt);
    // All but the last entry must be identical to the mean:
    for (size_t j = 0; j < mean.size() - 1; ++j) {
      CHECK_EQUAL(mean[j], x[j]);
    }
  } while(i++ < 3);

  C(0,0) = 0;
  C(1,1) = 1;
  C(n-1,n-1) = 0;
  mvn = multivariate_normal_distribution<decltype(mean)>(mean, C);
  i = 0;
  do {
    x = mvn(mt);
    for (size_t j = 0; j < mean.size() - 1; ++j) {
      if (j != 1) {
        CHECK_EQUAL(mean[j], x[j]);
      }
    }
  } while(i++ < 10);

  C(1,1) = 0;
  C(n-2,n-2) = 1;
  mvn = multivariate_normal_distribution<decltype(mean)>(mean, C);
  i = 0;
  do {
    x = mvn(mt);
    for (size_t j = 0; j < mean.size() - 1; ++j) {
      if (j != n-2) {
        CHECK_EQUAL(mean[j], x[j]);
      }
    }
  } while(i++ < 3);

  // Scaling test: If C->kC for some constant k, then A->sqrt(k)A.
  // First we build a random positive semidefinite matrix:
  Matrix<Real, Dynamic, Dynamic> C1 = Matrix<Real, Dynamic, Dynamic>::Random(n, n);
  C = C1.transpose()*C1;
  // Set the mean to 0:
  for (auto & m : mean) {
    m = 0;
  }
  samples = 1;
  std::vector<std::array<Real, n>> x1(samples);
  mt.seed(12859);
  mvn = multivariate_normal_distribution<decltype(mean)>(mean, C);
  for (size_t i = 0; i < x1.size(); ++i) {
    x1[i] = mvn(mt);
  }
  // Now scale C:
  C *= 16;
  // Set the seed back to the original:
  mt.seed(12859);
  std::vector<std::array<Real, n>> x2(samples);
  mvn = multivariate_normal_distribution<decltype(mean)>(mean, C);
  for (size_t i = 0; i < x2.size(); ++i) {
    x2[i] = mvn(mt);
  }
  // Now x2 = 4*x1 is expected:
  for (size_t i = 0; i < x1.size(); ++i) {
    for (size_t j = 0; j < n; ++j) {
      CHECK_ULP_CLOSE(4*x1[i][j], x2[i][j], 2);
    }
  }

}

int main() {
  test_multivariate_normal<float>();
  test_multivariate_normal<double>();
  return boost::math::test::report_errors();
}
