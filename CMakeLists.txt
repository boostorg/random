# Copyright 2019 Sam Day
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
#
# NOTE: CMake support for Boost.random is currently experimental at best
#       and the interface is likely to change in the future

cmake_minimum_required(VERSION 3.5)
project(BoostRandom LANGUAGES CXX)

add_library(boost_random
        src/random_device.cpp
        )

add_library(Boost::random ALIAS boost_random)

target_include_directories(boost_random PUBLIC include)

target_link_libraries(boost_random
        PUBLIC
        Boost::assert
        Boost::config
        Boost::core
        Boost::detail
        Boost::dynamic_bitset
        Boost::integer
        Boost::io
        Boost::math
        Boost::mpl
        Boost::multiprecision
        Boost::range
        Boost::system
        Boost::static_assert
        Boost::throw_exception
        Boost::type_traits
        Boost::utility
        )
