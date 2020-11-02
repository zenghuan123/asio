cmake_minimum_required(VERSION 3.8)
project(muduo_example)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS True)
find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
#include_directories(${muduo_path}/include)

list(APPEND CMAKE_CXX_FLAGS   "-pthread" )


#link_directories(${muduo_path}/lib)
set(muduo_lib libmuduo_net.a libmuduo_inspect.a libmuduo_http.a libmuduo_base.a)
add_executable(server server.cpp )
target_link_libraries(server ${muduo_lib})
add_executable(client client.cpp )
target_link_libraries(client ${muduo_lib})
