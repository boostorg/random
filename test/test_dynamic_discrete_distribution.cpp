/* test_dynamic_discrete_distribution.cpp
 *
 * Copyright Steven Watanabe 2010
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * $Id$
 *
 */

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/dynamic_discrete_distribution.hpp>
#include <sstream>
#include <vector>
#include<random>
#include "concepts.hpp"
#include "chi_squared_test.hpp"

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>



using boost::random::test::RandomNumberDistribution;
using boost::random::dynamic_discrete_distribution;

BOOST_CONCEPT_ASSERT((RandomNumberDistribution< dynamic_discrete_distribution<> >));

struct gen {
    double operator()(double arg) {
        if(arg < 100) return 100;
        else if(arg < 103) return 1;
        else if(arg < 107) return 2;
        else if(arg < 111) return 1;
        else if(arg < 114) return 4;
        else return 100;
    }
};

#define CHECK_PROBABILITIES(actual, expected)       \
    do {                                            \
        std::vector<double> _actual = (actual);     \
        std::vector<double> _expected = (expected); \
        BOOST_CHECK_EQUAL_COLLECTIONS(              \
            _actual.begin(), _actual.end(),         \
            _expected.begin(), _expected.end());    \
    } while(false)

//using boost::assign::list_of;

BOOST_AUTO_TEST_CASE(test_constructors) {
    boost::random::dynamic_discrete_distribution<> dist;
    CHECK_PROBABILITIES(dist.probabilities(),(std::vector<double>{1.0}));

#ifndef BOOST_NO_CXX11_HDR_INITIALIZER_LIST
    boost::random::dynamic_discrete_distribution<> dist_il = { 1, 2, 1, 4 };
    CHECK_PROBABILITIES(dist_il.probabilities(), (std::vector<double>{.125,.25,.125,.5}));
#endif
    std::vector<double> probs = {1.0,2.0,1.0,4.0};

    boost::random::dynamic_discrete_distribution<> dist_r(probs);
    CHECK_PROBABILITIES(dist_r.probabilities(), (std::vector<double>{.125,.25,.125,.5}));

    boost::random::dynamic_discrete_distribution<> dist_i({1.0,2.0,1.0,4.0});
    CHECK_PROBABILITIES(dist_r.probabilities(), (std::vector<double>{.125,.25,.125,.5}));
    
    boost::random::dynamic_discrete_distribution<> dist_it(probs.begin(), probs.end());
    CHECK_PROBABILITIES(dist_it.probabilities(), (std::vector<double>{.125,.25,.125,.5}));

    boost::random::dynamic_discrete_distribution<> dist_fun(4, 99, 115, gen());
    CHECK_PROBABILITIES(dist_fun.probabilities(), (std::vector<double>{.125,.25,.125,.5}));
    boost::random::dynamic_discrete_distribution<> copy(dist);
    BOOST_CHECK_EQUAL(dist, copy);
    boost::random::dynamic_discrete_distribution<> copy_r(dist_r);
    BOOST_CHECK_EQUAL(dist_r, copy_r);

   boost::random::dynamic_discrete_distribution<> notpow2(3, 99, 111, gen());
    BOOST_REQUIRE_EQUAL(notpow2.probabilities().size(), 3u);
    BOOST_CHECK_CLOSE_FRACTION(notpow2.probabilities()[0], 0.25, 0.00000000001);
    BOOST_CHECK_CLOSE_FRACTION(notpow2.probabilities()[1], 0.50, 0.00000000001);
    BOOST_CHECK_CLOSE_FRACTION(notpow2.probabilities()[2], 0.25, 0.00000000001);
    boost::random::dynamic_discrete_distribution<> copy_notpow2(notpow2);
    BOOST_CHECK_EQUAL(notpow2, copy_notpow2);
}

BOOST_AUTO_TEST_CASE(test_param) {
    std::vector<double> probs = {1.0,2.0,1.0,4.0};
    boost::random::dynamic_discrete_distribution<> dist(probs);
    boost::random::dynamic_discrete_distribution<>::param_type param = dist.param();
    CHECK_PROBABILITIES(param.probabilities(), (std::vector<double>{.125,.25,.125,.5}));
    boost::random::dynamic_discrete_distribution<> copy1(param);
    BOOST_CHECK_EQUAL(dist, copy1);
    boost::random::dynamic_discrete_distribution<> copy2;
    copy2.param(param);
    BOOST_CHECK_EQUAL(dist, copy2);

    boost::random::dynamic_discrete_distribution<>::param_type param_copy = param;
    BOOST_CHECK_EQUAL(param, param_copy);
    BOOST_CHECK(param == param_copy);
    BOOST_CHECK(!(param != param_copy));
    boost::random::dynamic_discrete_distribution<>::param_type param_default;
    CHECK_PROBABILITIES(param_default.probabilities(), (std::vector<double>{1.0}));
    BOOST_CHECK(param != param_default);
    BOOST_CHECK(!(param == param_default));
    
#ifndef BOOST_NO_CXX11_HDR_INITIALIZER_LIST
    boost::random::dynamic_discrete_distribution<>::param_type
        parm_il = (std::vector<double>{ 1, 2, 1, 4 });
    CHECK_PROBABILITIES(parm_il.probabilities(), (std::vector<double>{.125,.25,.125,.5}));
#endif

    boost::random::dynamic_discrete_distribution<>::param_type parm_r(probs);
    CHECK_PROBABILITIES(parm_r.probabilities(), (std::vector<double>{.125,.25,.125,.5}));
    
    boost::random::dynamic_discrete_distribution<>::param_type
        parm_it(probs.begin(), probs.end());
    CHECK_PROBABILITIES(parm_it.probabilities(), (std::vector<double>{.125,.25,.125,.5}));

    boost::random::dynamic_discrete_distribution<>::param_type
        parm_fun(4, 99, 115, gen());
    CHECK_PROBABILITIES(parm_fun.probabilities(), (std::vector<double>{.125,.25,.125,.5}));
  
}

BOOST_AUTO_TEST_CASE(test_min_max) {
    std::vector<double> probs = {1.0,2.0,1.0};
    boost::random::dynamic_discrete_distribution<> dist;
    BOOST_CHECK_EQUAL((dist.min)(), 0);
    BOOST_CHECK_EQUAL((dist.max)(), 0);
    boost::random::dynamic_discrete_distribution<> dist_r(probs);
    BOOST_CHECK_EQUAL((dist_r.min)(), 0);
    BOOST_CHECK_EQUAL((dist_r.max)(), 2);
}

BOOST_AUTO_TEST_CASE(test_comparison) {
    std::vector<double> probs = {1.0,2.0,1.0,4.0};
    boost::random::dynamic_discrete_distribution<> dist;
    boost::random::dynamic_discrete_distribution<> dist_copy(dist);
    boost::random::dynamic_discrete_distribution<> dist_r(probs);
    boost::random::dynamic_discrete_distribution<> dist_r_copy(dist_r);
    BOOST_CHECK(dist == dist_copy);
    BOOST_CHECK(!(dist != dist_copy));
    BOOST_CHECK(dist_r == dist_r_copy);
    BOOST_CHECK(!(dist_r != dist_r_copy));
    BOOST_CHECK(dist != dist_r);
    BOOST_CHECK(!(dist == dist_r));
}

BOOST_AUTO_TEST_CASE(test_streaming) {
    std::vector<double> probs = {1.0,2.0,1.0,4.0};
    boost::random::dynamic_discrete_distribution<> dist(probs);
    std::stringstream stream;
    stream << dist;
    boost::random::dynamic_discrete_distribution<> restored_dist;
    stream >> restored_dist;
    BOOST_CHECK_EQUAL(dist, restored_dist);
}

BOOST_AUTO_TEST_CASE(test_generation) {
    std::vector<double> probs = {0.0,1.0};
    std::minstd_rand0 gen;
    boost::random::dynamic_discrete_distribution<> dist;
    boost::random::dynamic_discrete_distribution<> dist_r(probs);
    for(int i = 0; i < 10; ++i) {
        int value = dist(gen);
        BOOST_CHECK_EQUAL(value, 0);
        int value_r = dist_r(gen);
        BOOST_CHECK_EQUAL(value_r, 1);
        int value_param = dist_r(gen, dist.param());
        BOOST_CHECK_EQUAL(value_param, 0);
        int value_r_param = dist(gen, dist_r.param());
        BOOST_CHECK_EQUAL(value_r_param, 1);
    }
}

template<int k>
double testSelection( int n, int max){
    std::uniform_real_distribution uniform_real_dist(.5,1.0);

    max = std::min(max,10*n);
    boost::mt19937 generator;
    std::vector<double> weights(n);
    double sum = 0;
    for (int i=0;i<n;i++){
        double rand = uniform_real_dist(generator);
        weights[i] = rand;
        sum+=rand;
    }
    for (int i=0;i<n;i++){
        weights[i]=weights[i]/sum;
    }

    boost::random::dynamic_discrete_distribution<size_t,double,k> dist(weights);
    std::vector<int> results(n,0);
    for (int i=0;i<max;i++){
        results[dist(generator)]+=1;
    }

    double result = chi_squared_test(results, weights, max);
    return result;


}

template <int k>
double testUpdate( int n, int max){
    std::uniform_real_distribution uniform_real_dist(.5,1.0);

    max = std::min(max,10*n);
    boost::mt19937 generator;
    std::vector<double> weights(n);
    double sum = 0;
    for (int i=0;i<n;i++){
        double rand = uniform_real_dist(generator);
        weights[i] = rand;
        sum+=rand;
    }

    boost::random::dynamic_discrete_distribution<size_t,double,k> dist(weights);
    std::uniform_int_distribution uniform_dist(0,n-1);
    for (int i=0;i<n;i++){
        int changeIndex = uniform_dist(generator);
        double newWeight = generator();
        sum -= weights[changeIndex];
        sum += newWeight;
        dist.update_weight(changeIndex,newWeight);
        weights[changeIndex] = newWeight;


    }
    for (int i=0;i<n;i++){
        weights[i]=weights[i]/sum;
    }
    std::vector<int> results(n,0);
    for (int i=0;i<max;i++){
        results[dist(generator)]+=1;
    }

    return chi_squared_test(results, weights, max);

}

// template<class intType, class Real, int k>
// bool stat_test(boost::random::dynamic_discrete_distribution<intType,Real,k> distrubiton, std::vector<Real> weights){
//      boost::mt19937 generator;
//      auto probabilities = weights;
//      int sum = 0;
//      for (int i=0; i<weights.size();i++){
//         sum += weights[i];
//      }
//      for (int i=0; i<weights.size();i++){

//      }

// }

BOOST_AUTO_TEST_CASE(get_weight){
    boost::random::dynamic_discrete_distribution<size_t,double,4> dist({0,1,2,3,4,5});
    BOOST_CHECK(dist.get_weight(0)==0);
    BOOST_CHECK(dist.get_weight(5)==5);

}

//Question should we throw errors when container is empty?
//Question, when container is empty, should we throw errors during selection?
BOOST_AUTO_TEST_CASE(adding_removing){
    boost::random::dynamic_discrete_distribution<size_t,double,4> dist{0,1,2,3,4};
    dist.pop_back(5);
    BOOST_CHECK(dist.total_weight()==0);

    std::uniform_real_distribution uniform_real_dist(.5,1.0);
    boost::mt19937 generator;
    std::vector<double> weights(35);
    double sum = 0;
    for (int i=0;i<35;i++){
        double rand = uniform_real_dist(generator);
        dist.push_back(rand);
        weights[i] = rand;
        sum+=rand;
    }
    for (size_t i=0;i<weights.size();i++){
        weights[i]=weights[i]/sum;
    }

    BOOST_CHECK(dist.total_weight()==sum);

    std::vector<int> results(35);
    int iters = 1000;
    for (int i=0;i<iters;i++){
        results[dist(generator)]+=1;
    }

    double result = chi_squared_test(results, weights, iters);
    BOOST_CHECK(result<0.99);

    sum=1;
    for (int i=0; i<25;i++){
        dist.pop_back();
        sum-=weights[weights.size()-1];
        weights.pop_back();
    }
    for (size_t i=0; i<weights.size();i++){
        weights[i] /=sum;
    }
    std::vector<int> newResults(weights.size());
    for (int i=0; i<iters;i++){
        newResults[dist(generator)]+=1;
    }
    BOOST_CHECK(chi_squared_test(newResults,weights,iters)<0.99);

    
}

BOOST_AUTO_TEST_CASE(selection){
    //k=2 
    BOOST_CHECK(testSelection<2>(2,1000)<0.99);
    BOOST_CHECK(testSelection<2>(3,1000)<0.99);
    BOOST_CHECK(testSelection<2>(30,100000)<0.99);
    //k=8
    BOOST_CHECK(testSelection<8>(2,1000)<0.99);
    BOOST_CHECK(testSelection<8>(9,1000)<0.99);
    BOOST_CHECK(testSelection<8>(100,100000)<0.99);
    //k= 32
    BOOST_CHECK(testSelection<32>(2,1000)<0.99);
    BOOST_CHECK(testSelection<32>(33,1000)<0.99);
    BOOST_CHECK(testSelection<32>(1000,1000000)<0.99);

}

BOOST_AUTO_TEST_CASE(update){
    //k=2 
    BOOST_CHECK(testUpdate<2>(2,1000)<0.99);
    BOOST_CHECK(testUpdate<2>(3,1000)<0.99);
    BOOST_CHECK(testUpdate<2>(30,100000)<0.99);
    //k=8
    BOOST_CHECK(testUpdate<8>(2,1000)<0.99);
    BOOST_CHECK(testUpdate<8>(9,1000)<0.99);
    BOOST_CHECK(testUpdate<8>(100,10000)<0.99);
    //k= 32
    BOOST_CHECK(testUpdate<32>(2,1000)<0.99);
    BOOST_CHECK(testUpdate<32>(33,100000)<0.99);
    BOOST_CHECK(testUpdate<32>(1000,100000)<0.99);

}
