// mixmax_demo.cpp
//
// Copyright (c) 2019
// Kostas Savvidis
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


/*
    For the source of this example see
    [@boost://libs/random/example/mixmax_demo.cpp mixmax_demo.cpp].
    First we include the headers we need for mixmax
    and __uniform_real_distribution.
*/
#include <iostream>
#include <boost/random/mixmax.hpp>
#include <boost/random/uniform_real_distribution.hpp>

boost::random::mixmax gen(0,0,0,1); // Create a Mixmax object and initialize the RNG with four 32-bit seeds 0,0,0,1

double flat() {
    boost::random::uniform_real_distribution<> dist(0,1);
    /*<< A distribution is a function object.  We generate a random
        number by calling `dist` with the generator.
    >>*/
    
    //return double(gen())*(1.0/gen.max()); // 5% faster, why is random::uniform_real_distribution with dist(0,1) not specialized to this?
    //return gen++; // 10% faster, built-in method is hand-optimized
    return dist(gen);
}



int main() {
    std::cerr << "Welcome to the MIXMAX random number generator!" 
    << "\n The curent matrix size is N=" << gen.rng_get_N() 
    << "\n (the actual matrix is not kept in memory in this new efficient implementation)"
    << "\n special entry in the matrix is " << gen.rng_get_SPECIAL()
    << "\n special multiplier m=2^" << gen.rng_get_SPECIALMUL()<<"+1"
    << "\n Working in the Galois field with prime modulus 2^61-1"
    << "\n Generator class size is "<< sizeof(gen) << " bytes\n\n";

	std::cerr << "\nFirst print a dozen doubles on [0,1] using the boost::random::uniform_real_distribution\n";
	std::cout.setf(std::ios::fixed);
    for(int i = 0; i < 12; ++i) {
        std::cout << flat() << std::endl;
    }
    
    //gen.discard(100); // throw away 100 values
    
    std::cerr << "\nNow add one billion doubles on [0,1] using the built-in method get_next_float()\n";
    int p = 1000000000;
    double z=0.0;
    for (int j=0; j<p ; j++) {
        //z += gen++;    // why not? overloaded postfix ++ increments state by one step, returns a double
        //z += flat(); // about 10% slower, due to the overhead of random::uniform_real_distribution
    }
    printf("\n%1.16F\n", z);
    std::cout << "ok\n\n";
    
    std::cout << "Now, save the state of the generator to stdout:\n";
    std::cout << gen;
	std::cout << "Now, read the state of the generator from stdin:\n";
	boost::random::mixmax gen2;
	std::cin >> gen2;
	p=100; do{if(gen++==gen2++) {std::cout << "OK, same state\n";}else{std::cout << "NOT OK";}  }while(p--!=0);
    return 0;
}
