/*
 * Copyright Matt Borland 2022.
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <cstdint>
#include <random>
#include <boost/random/random_device.hpp>
#include <benchmark/benchmark.h>
#include "../include/boost/random/splitmix64.hpp"

template <typename T>
void stl_random_device(benchmark::State& state)
{
    std::random_device rd;
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(rd());
    }
}

template <typename T>
void boost_random_device(benchmark::State& state)
{
    boost::random::random_device rd;
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(rd());
    }
}

template <typename T>
void boost_splitmix64(benchmark::State& state)
{
    boost::random::splitmix64 rd;
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(rd());
    }
}

BENCHMARK_TEMPLATE(stl_random_device, unsigned);
BENCHMARK_TEMPLATE(boost_random_device, unsigned);
BENCHMARK_TEMPLATE(boost_splitmix64, std::uint64_t);

BENCHMARK_MAIN();
