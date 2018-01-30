cmake_minimum_required(VERSION 3.9)
project(Algorithm)

set(CMAKE_C_STANDARD 99)

add_executable(Algorithm Algorithm.cpp Algorithm.hpp)

target_link_libraries(Algorithm opencv2)

install(TARGETS Algorithm DESTINATION bin