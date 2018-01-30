cmake_minimum_required(VERSION 3.9)
project(Algorithm)
set(CMAKE_C_STANDARD 99)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(Algorithm Algorithm.cpp Algorithm.hpp)

target_link_libraries(Algorithm ${OpenCV_LIBS})

install(TARGETS Algorithm DESTINATION bin)