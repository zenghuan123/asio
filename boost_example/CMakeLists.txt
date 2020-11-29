CMAKE_MINIMUM_REQUIRED(VERSION 3.7.0)

PROJECT(boost_example VERSION 1.0)


SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED True)


set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         ON) # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

set(boost_module 
		"date_time"
		"chrono"
		)

FIND_PACKAGE(Boost REQUIRED COMPONENTS ${boost_module})
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
message(STATUS  ${Boost_LIBRARIES})
LINK_LIBRARIES(${Boost_LIBRARIES})
ADD_DEFINITIONS( "-DHAS_BOOST" )

FILE(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

file(GLOB_RECURSE CURRENT_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
source_group("Include" FILES ${CURRENT_HEADERS})

ADD_EXECUTABLE(example ${SRC} ${CURRENT_HEADERS})
TARGET_INCLUDE_DIRECTORIES(example PUBLIC  "include")
