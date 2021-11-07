cmake_minimum_required(VERSION 2.8)
project(chromatyna)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_library(BOOST_PROGRAM_OPTIONS boost_program_options)

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -pedantic -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_BUILD_TYPE RELEASE)

set(PATHDECOMP_SOURCE_FILES
        src/path-decomposition.cpp src/namesdict.h src/pathdecomp.h)

set(RAW_SOURCE_FILES
        src/parse_raw.cpp)

set(KNOTFINDER_SOURCE_FILES
        src/pathdecomp.h src/namesdict.h src/find-knots.cpp src/tools.h src/graph.h src/minorfinder.h src/minorfinder.cpp)

set(LINEARFINDER_SOURCE_FILES
        src/namesdict.h src/find-k6-linear.cpp src/tools.h src/graph.h)

add_executable(path-decomposition ${PATHDECOMP_SOURCE_FILES})
target_link_libraries(path-decomposition ${BOOST_PROGRAM_OPTIONS})
add_executable(parse_raw ${RAW_SOURCE_FILES})
target_link_libraries(parse_raw ${BOOST_PROGRAM_OPTIONS})
add_executable(find-knots ${KNOTFINDER_SOURCE_FILES})
target_link_libraries(find-knots ${BOOST_PROGRAM_OPTIONS})
add_executable(find-k6-linear ${LINEARFINDER_SOURCE_FILES})
target_link_libraries(find-k6-linear ${BOOST_PROGRAM_OPTIONS})
