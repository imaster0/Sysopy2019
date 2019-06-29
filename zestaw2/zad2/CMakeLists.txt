cmake_minimum_required(VERSION 2.8)

project(zad2 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

add_executable(main main.c)

