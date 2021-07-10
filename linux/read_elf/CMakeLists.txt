
add_library(t1 SHARED test1.cpp)
add_library(t2 SHARED test2.cpp)

add_executable(read_elf main.cpp)

target_link_libraries(read_elf t1 t2)