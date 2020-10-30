cmake_minimum_required(VERSION 3.8)
project(muduo_example)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS True)
find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${muduo_path}/include)

list(APPEND CMAKE_CXX_FLAGS   "-pthread" )
add_subdirectory(muduo)
