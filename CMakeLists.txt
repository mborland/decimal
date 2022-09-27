# Copyright 2020 Peter Dimov
# Copyright 2022 Matt Borland
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.12)

project(boost_decimal VERSION "${BOOST_SUPERPROJECT_VERSION}" LANGUAGES CXX)

add_library(boost_decimal INTERFACE)

add_library(Boost::decimal ALIAS boost_decimal)

target_include_directories(boost_decimal INTERFACE include)

set_property(TARGET boost_decimal PROPERTY CXX_STANDARD 20)

include(CMakeDependentOption)

cmake_dependent_option(BOOST_DECIMAL_STANDALONE "Use Boost.Decimal in standalone mode" ON "NOT BOOST_SUPERPROJECT_VERSION" OFF)

message(STATUS "Boost.Decimal: standalone mode ${BOOST_DECIMAL_STANDALONE}")

if(BOOST_MATH_STANDALONE)

  target_compile_definitions(boost_decimal INTERFACE BOOST_MATH_STANDALONE=1)

else()

  target_link_libraries(boost_decimal
    INTERFACE
      Boost::config
  )

endif()

# Only enable tests when we're the root project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)

  include(CTest)
  add_subdirectory(test)

endif()
