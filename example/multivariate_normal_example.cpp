/* boost random_test.cpp various tests
 *
 * Copyright Matthias Troyer 2006
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * $Id$
 */

#include <boost/random/multivariate_normal_distribution.hpp>
#include <boost/random.hpp>
#include <iostream>

int main()
{

  // Create multivariate correlated Gaussians with mean (-1, 1) and covariance
  // matrix ((2,2),(2,2))
  
  typedef boost::multivariate_normal_distribution<double> distribution_type;
  
  // the square root (Cholesky decomposition) of the covariance matrix
  distribution_type::matrix_type cholesky(2,2);
  cholesky(0,0)=1.;
  cholesky(0,1)=1.;
  cholesky(1,0)=1.;
  cholesky(1,1)=1.;
  
  // the vector of mean values
  distribution_type::vector_type mean(2);
  mean(0)=-1.;
  mean(1)=1.;
  
  // create the engine, distribution, and variate generator
  boost::mt19937 engine;
  distribution_type dist(cholesky,mean);
  boost::variate_generator<boost::mt19937,distribution_type>
    gen(engine,dist);
    
  // print 100 pairs of correlated Gaussians
  for (int i=0; i<100; ++i)
    std::cout << gen() << " " << gen() << "\n";
}
