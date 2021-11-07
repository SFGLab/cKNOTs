cmake_minimum_required(VERSION 2.8)
project(splitter)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

find_library(BOOST_PROGRAM_OPTIONS boost_program_options)

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -pedantic -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_BUILD_TYPE DEBUG)

set(SPLITTER splitter.cpp)

add_executable(splitter ${SPLITTER})
target_link_libraries(splitter ${BOOST_PROGRAM_OPTIONS})
