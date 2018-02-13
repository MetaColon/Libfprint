cmake_minimum_required(VERSION 3.9)
project(Algorithm)
set(CMAKE_C_STANDARD 99)

find_package(OpenCV REQUIRED)
find_package(libfprint REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(Algorithm Algorithm.cpp Algorithm.h Testing.cpp Testing.h Testing.cpp Testing.h)

target_link_libraries(Algorithm ${OpenCV_LIBS})

target_link_libraries(Algorithm libfprint)

install(TARGETS Algorithm DESTINATION bin)