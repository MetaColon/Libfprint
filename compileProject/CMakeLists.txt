cmake_minimum_required(VERSION 3.5)
project(compileProject)

set(CMAKE_C_STANDARD 99)

add_executable(compileProject main.c)

target_link_libraries(compileProject usb-1.0)

install(TARGETS compileProject DESTINATION bin)