cmake_minimum_required(VERSION 2.8)

project(zad2 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC ")

add_library(my_library_static STATIC ../zad1/library.c)
add_library(my_library_shared SHARED ../zad1/library.c)

add_executable(main_static main.c)
add_executable(main_shared main.c)

target_link_libraries(main_static my_library_static)
target_link_libraries(main_shared my_library_shared)

