//
// Copyright (c) 2017 James E. King III
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//   http://www.boost.org/LICENCE_1_0.txt)
//
// benchmark for random_generators in different forms
//

#include <boost/core/ignore_unused.hpp>
#include <boost/timer/timer.hpp>
#include <boost/predef/os.h>
#include <boost/random/random_device.hpp>
#include <iostream>
#include <limits>

#if defined(BOOST_RANDOM_LIMITED_BENCH)
// must be a Valgrind, UBsan, or other stressful check job
#define GEN_LOOPS 10
#define REUSE_LOOPS 100
#else
#define GEN_LOOPS 10000
#define REUSE_LOOPS 1000000
#endif

template<class Provider>
void auto_timed_ctordtor(size_t count)
{
    boost::timer::auto_cpu_timer t;
    for (size_t i = 0; i < count; ++i)
    {
        Provider gen;
        boost::ignore_unused(gen);
    }
}

template<class Provider>
void auto_timed_bench(size_t count)
{
    Provider gen;
    {
        boost::timer::auto_cpu_timer t;
        for (size_t i = 0; i < count; ++i)
        {
            typename Provider::result_type u = gen();
            boost::ignore_unused(u);
        }
    }
}

int main(int, char*[])
{
    std::cout << "Operating system entropy provider: "
              << boost::random::random_device().name() << std::endl;

    std::cout << "Construction/destruction (overhead) time for boost::random_device "
        << "(" << GEN_LOOPS << " iterations): " << std::endl;
    auto_timed_ctordtor<boost::random_device>(GEN_LOOPS);
    std::cout << std::endl;

    std::cout << "Benchmark boost:::random_device operator()()"
              << "(reused for " << REUSE_LOOPS << " loops):" << std::endl;
    auto_timed_bench<boost::random_device>(REUSE_LOOPS);
    std::cout << std::endl;

    return 0;
}

