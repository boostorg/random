#define BOOST_TEST_MAIN

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/negative_binomial_distribution.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_fractional_negative_binomial) {
  // Draw sample_size values from NB(k, p). k is chosen so that if it were
  // rounded to an integer, the sample mean would be very different.
  const size_t sample_size = 1000;
  const double k = 0.5;
  const double p = 0.1;
  boost::random::negative_binomial_distribution<> dist(k, p);
  boost::random::mt19937 rng(17);  // arbitrary seed
  double sum = 0.0;
  for (size_t i = 0; i < sample_size; i++) {
    sum += dist(rng);
  }
  double sample_mean = sum / sample_size;
  // https://en.wikipedia.org/w/index.php?title=Negative_binomial_distribution&oldid=849463302
  // has these formulae, though Wikipedia's (r, p) maps to Boost's (k, 1-p).
  const double expected_mean = k * (1 - p) / p;
  const double expected_sd  = std::sqrt(k * (1 - p) / p / p);
  // Check that the sample mean is within two standard errors (for roughly a
  // 95% confidence) of the expectation.
  const double standard_error = expected_sd / std::sqrt(sample_size);
  BOOST_CHECK_LT(std::abs(expected_mean - sample_mean) / standard_error, 2.0);
}
