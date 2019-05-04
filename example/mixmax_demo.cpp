// driver_mixmax.cpp
//
// Copyright (c) 2017
// Kostas Savvidis
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


/*
    For the source of this example see
    [@boost://libs/random/example/driver_mixmax.cpp driver_mixmax.cpp].
    First we include the headers we need for mixmax
    and __uniform_real_distribution.
*/
#include <iostream>
#include <boost/random/mixmax.hpp>
#include <boost/random/uniform_real_distribution.hpp>

/*`
  We use mixmax with the default seed as a source of
  randomness.  The numbers produced will be the same
  every time the program is run.  One common method to
  change this is to seed with the current time (`std::time(0)`
  defined in ctime).
*/
boost::random::mixmax gen(0,0,0,1);
/*`
  [note We are using a /global/ generator object here.  This
  is important because we don't want to create a new [prng
  pseudo-random number generator] at every call]
*/
/*`
  Now we can define a function that simulates an ordinary
  six-sided die.
*/
double roll() {
    boost::random::uniform_real_distribution<> dist(0,1);
    /*<< A distribution is a function object.  We generate a random
        number by calling `dist` with the generator.
    >>*/
    return dist(gen);
}
//]


int main() {
    for(int i = 0; i < 24; ++i) {
        std::cout << roll() << std::endl;
    }
}
