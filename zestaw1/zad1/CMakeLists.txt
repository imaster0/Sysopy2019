cmake_minimum_required(VERSION 2.8)

project(zad1 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_library(my_library_static STATIC library.c)
add_library(my_library_shared SHARED library.c)

