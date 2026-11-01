# Copyright 2021-2026 Alexander Grund
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.5...3.31)

project(cmake_subdir_test LANGUAGES CXX)

# Those 2 should work the same
# while using find_package for the installed Boost avoids the need to manually specify dependencies
if(BOOST_CI_INSTALL_TEST)
    # Regular usage:
    # Provides target Boost::random
    find_package(Boost COMPONENTS random REQUIRED)
else()
    add_subdirectory(../.. boostorg/random)

    set(deps
      # Primary dependencies
      assert
      config
      core
      dynamic_bitset
      integer
      io
      system
      throw_exception
      type_traits
      utility
      # Secondary dependencies
      compat
      container_hash
      describe
      mp11
      predef
      preprocessor
      variant2
      winapi
    )

    foreach(dep IN LISTS deps)
      add_subdirectory(../../../${dep} boostorg/${dep})
    endforeach()
endif()

add_executable(main main.cpp)
target_link_libraries(main Boost::random)

enable_testing()
add_test(NAME main COMMAND main)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>)
