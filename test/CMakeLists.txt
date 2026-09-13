# Copyright 2018, 2019 Peter Dimov
# Copyright 2023 Matt Borland
# Copyright 2026 Alexander Grund
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

include(BoostTest OPTIONAL RESULT_VARIABLE HAVE_BOOST_TEST)

if(HAVE_BOOST_TEST)

  file(STRINGS "Jamfile.v2" data)

  foreach(line IN LISTS data)
    if(line MATCHES "^[ \t]*run[ \t]+(.+)")
      set(args "${CMAKE_MATCH_1}")
      if(args MATCHES "multiprecision_.+_test")
        continue()
      elseif(NOT args MATCHES "^([^:;]+)[ \t]*;[ \t]*$")
        boost_message(AUTHOR_WARNING "boost_test_jamfile: Unknown Jamfile line: ${line}\n'${args}'")
        continue()
      endif()
      string(REPLACE " " ";" parts "${CMAKE_MATCH_1}")
      set(sources )
      set(extra_libs )
      foreach(part IN ITEMS ${parts})
        if(part MATCHES "^[./a-zA-Z0-9_]+\.cpp+$")
            list(APPEND sources ${part})
        elseif(part STREQUAL "/boost/assign//boost_assign")
            list(APPEND extra_libs Boost::assign)
        elseif(part STREQUAL "/boost/iterator//boost_iterator")
            list(APPEND extra_libs Boost::iterator)
        elseif(part STREQUAL "/boost/test//boost_unit_test_framework")
            list(APPEND extra_libs Boost::unit_test_framework)
        elseif(part STREQUAL "math_test")
            list(APPEND extra_libs Boost::assign Boost::bind Boost::exception Boost::lexical_cast Boost::math Boost::numeric_conversion)
        else()
            boost_message(SEND_ERROR "Unknown dependency '${part}'")
        endif()
      endforeach()
      if(sources MATCHES "^test_lagged_fibonacci[0-9]+")
        # Larger ones overflow the stack, see comment in Jamfile
        set(type compile)
      else()
        set(type run)
      endif()
      boost_test(TYPE ${type} SOURCES ${sources} LINK_LIBRARIES Boost::random ${extra_libs})
    endif()
  endforeach()
endif()

boost_test(TYPE run SOURCES multiprecision_int_test.cpp
           LINK_LIBRARIES Boost::random Boost::unit_test_framework Boost::multiprecision
           COMPILE_FEATURES cxx_std_14)
boost_test(TYPE compile SOURCES multiprecision_float_test.cpp
           LINK_LIBRARIES Boost::random Boost::unit_test_framework Boost::multiprecision
           COMPILE_FEATURES cxx_std_14)
boost_test(TYPE compile SOURCES statistic_tests.cpp
           LINK_LIBRARIES Boost::random Boost::bind Boost::math)
