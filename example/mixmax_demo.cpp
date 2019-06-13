// mixmax_demo.cpp : g++-mp-8 -std=c++11 -O3 mixmax_demo.cpp -I /usr/local/include/
//
// Copyright (c) 2019
// Kostas Savvidis
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


/**
    For the source of this example see
    [@boost://libs/random/example/mixmax_demo.cpp mixmax_demo.cpp].
    First we include the headers we need for mixmax
    and __uniform_real_distribution.
*/
#include <iostream>
#include <boost/random/mixmax.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>

boost::random::mixmax gen;            // Create a Mixmax object and initialize the RNG with four 32-bit seeds 0,0,0,1

boost::random::uniform_real_distribution<> dist(0,1);
/*<< A distribution is a function object.  We generate a random
 number by calling `dist` with the generator.
 >>*/

int main() {
    std::cerr << "\n MIXMAX Generator with N="<< gen.N << " has the class size equal to "<< sizeof(gen) << " bytes\n\n";

    gen.discard(9999); // throw away 9999 values
    std::cout << "the 10000th value is: " << gen() << "\n";
    
    std::cout << "\n\nFirst print a dozen doubles on [0,1] using the boost::random::uniform_real_distribution\n";
    std::cout.setf(std::ios::fixed);
    for(int i = 0; i < 12; ++i) {
        std::cout << dist(gen) << std::endl;
    }
    
    
    std::cerr << "\nNow add one billion doubles on [0,1] :\n";
    int p = 1000000000;
    double z=0.0;
    for (int j=0; j<p ; j++) {
        z += dist(gen);
    }
    printf("\nSUM=%1.16F\n", z);
    std::cout << "ok\n\n";
    
//    std::cout << "Now, save the state of the generator to stdout:\n";
//    std::cout << gen;
//    std::cout << "Now, read the state of the generator from stdin:\n(you may copy&paste the above)";
//    boost::random::mixmax gen2;
//    std::cin >> gen2;
//    p=10; do{if(gen++==gen2++) {std::cout << "OK, same state\n";}else{std::cout << "NOT OK\n";}  }while(p--!=0);
    return 0;
}
