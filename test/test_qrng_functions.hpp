// Copyright Justinas Vygintas Daugmaudis, 2010-2017.
// Use, modification and distribution is subject to the
// Boost Software License, Version 1.0. (See accompanying
// file LICENSE-1.0 or http://www.boost.org/LICENSE-1.0)

#ifndef TEST_QRNG_FUNCTIONS_HPP_INCLUDED
#define TEST_QRNG_FUNCTIONS_HPP_INCLUDED

#include <boost/random/uniform_real.hpp>
#include <boost/test/floating_point_comparison.hpp>

namespace test {

// Invokes operator() precisely n times. This is to check that
// Engine::discard(n) actually has the same effect.
template<typename Engine>
inline void trivial_discard(Engine& eng, std::size_t n)
{
  for( ; n != 0; --n ) eng();
}


template<typename Engine, typename T, std::size_t Dimension>
inline void match_vector(Engine& eng, T (&pt)[Dimension])
{
  BOOST_REQUIRE_EQUAL( eng.dimension(), Dimension ); // paranoid check

  boost::uniform_real<T> dist;

  for( std::size_t i = 0; i != eng.dimension(); ++i )
  {
    T val = dist(eng);
    // We want to check that quasi-random number generator values differ no
    // more than 0.0006% of their value.
    BOOST_CHECK_CLOSE(pt[i], val, 0.0006);
  }
}


template<typename Engine, typename T, std::size_t Dimension, std::size_t N>
inline void expected_values(T (&pt)[N][Dimension], std::size_t skip)
{
  Engine eng(Dimension);

  eng.seed(skip);

  for( std::size_t i = 0; i != N; ++i )
    match_vector(eng, pt[i]);
}

template<typename Engine, typename T>
inline void test_zero_seed(std::size_t dimension)
{
  Engine eng(dimension);

  Engine other(dimension);
  other.seed(0);

  // Check that states are equal after zero seed.
  boost::uniform_real<T> dist;
  BOOST_CHECK( eng == other );
  for( std:: size_t i = 0; i != dimension; ++i )
  {
    T q_val = dist(eng);
    T t_val = dist(other);
    BOOST_CHECK_CLOSE(q_val, t_val, 0.0001);
  }
}

template<typename Engine, typename T, std::size_t Dimension, std::size_t N>
inline void seed_function(T (&pt)[N][Dimension], std::size_t skip)
{
  // Test zero seed before doing other tests.
  test_zero_seed<Engine, T>(Dimension);

  Engine eng(Dimension);
  for( std::size_t i = 0; i != N; ++i )
  {
    // For all N seeds an engine
    // and checks if the expected values match.
    eng.seed(skip + i);
    match_vector(eng, pt[i]);
  }
}

template<typename Engine, typename T, std::size_t Dimension, std::size_t N>
inline void discard_function(T (&pt)[N][Dimension], std::size_t skip)
{
  Engine eng(Dimension), trivial(Dimension), initial_state(Dimension);
  boost::uniform_real<T> dist;

  const std::size_t element_count = N * Dimension;
  const T* pt_array = reinterpret_cast<T *>(boost::addressof(pt));

  initial_state.seed(skip);
  for( std::size_t step = 0; step != element_count; ++step )
  {
    // Init to the same state
    eng = initial_state;
    trivial = initial_state;

    // Discards have to have the same effect
    eng.discard(step);
    trivial_discard(trivial, step);

    // Therefore, states are equal
    BOOST_CHECK( eng == trivial );

    // Now, let's check whether they really produce the same sequence
    for( std::size_t k = step; k != element_count; ++k )
    {
      T q_val = dist(eng);
      T t_val = dist(trivial);
      BOOST_CHECK_CLOSE(q_val, t_val, 0.0001);
      // ~ BOOST_CHECK(q_val == t_val), but those are floating point values,
      // so strict equality check may fail unnecessarily

      // States remain equal!
      BOOST_CHECK( eng == trivial );

      // We want to check that quasi-random number generator values differ no
      // more than 0.0006% of their value.
      BOOST_CHECK_CLOSE(pt_array[k], q_val, 0.0006);
    }
  }
}

inline bool accept_all_exceptions(const std::exception& e)
{
  BOOST_TEST_MESSAGE( e.what() );
  return true;
}

} // namespace test


#define QRNG_VALIDATION_TEST_FUNCTIONS(QRNG) \
\
template<typename T, std::size_t Dimension, std::size_t N> \
inline void test_##QRNG##_values(T (&pt)[N][Dimension], std::size_t skip) \
{ \
  typedef boost::random::QRNG<> engine_t; \
  test::expected_values<engine_t>(pt, skip); \
} \
\
template<typename T, std::size_t Dimension, std::size_t N> \
inline void test_##QRNG##_seed(T (&pt)[N][Dimension], std::size_t skip) \
{ \
  typedef boost::random::QRNG<> engine_t; \
  test::seed_function<engine_t>(pt, skip); \
} \
\
template<typename T, std::size_t Dimension, std::size_t N> \
inline void test_##QRNG##_discard(T (&pt)[N][Dimension], std::size_t skip) \
{ \
  typedef boost::random::QRNG<> engine_t; \
  test::discard_function<engine_t>(pt, skip); \
} \
\
BOOST_AUTO_TEST_CASE( test_##QRNG##_zero_dimension_fails ) \
{ \
  typedef boost::random::QRNG<> engine_t; \
  BOOST_REQUIRE_EXCEPTION( engine_t(0), std::invalid_argument, test::accept_all_exceptions ) \
} \
\
BOOST_AUTO_TEST_CASE( test_##QRNG##_max_dimension_fails ) \
{ \
  typedef boost::random::QRNG<> engine_t; \
  BOOST_REQUIRE_EXCEPTION( engine_t(100000), std::invalid_argument, test::accept_all_exceptions ) \
} \
/**/

#endif // TEST_QRNG_FUNCTIONS_HPP_INCLUDED
