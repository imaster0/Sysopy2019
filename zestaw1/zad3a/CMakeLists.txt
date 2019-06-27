cmake_minimum_required(VERSION 2.8)

project(zad3a C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC ")

add_library(my_library_static STATIC ../zad1/library.c)
add_library(my_library_shared SHARED ../zad1/library.c)
add_library(my_library_dynamic MODULE ../zad1/library.c)

add_executable(main_static ../zad2/main.c)
add_executable(main_shared ../zad2/main.c)
add_executable(main_dynamic main_dynamic.c)

target_link_libraries(main_static my_library_static)
target_link_libraries(main_shared my_library_shared)
target_link_libraries(main_dynamic ${CMAKE_DL_LIBS})

enable_testing()

add_test(NAME static_test1 COMMAND main_static search_file_test 0 search_file_test 1 search_file_test 2)
add_test(NAME static_test2 COMMAND main_static save_blocks_test 0 save_blocks_test 1 save_blocks_test 2)
add_test(NAME static_test3 COMMAND main_static delete_blocks_test 0 delete_blocks_test 1 delete_blocks_test 2)
add_test(NAME static_test4 COMMAND main_static add_and_delete_test)

add_test(NAME shared_test1 COMMAND main_shared search_file_test 0 search_file_test 1 search_file_test 2)
add_test(NAME shared_test2 COMMAND main_shared save_blocks_test 0 save_blocks_test 1 save_blocks_test 2)
add_test(NAME shared_test3 COMMAND main_shared delete_blocks_test 0 delete_blocks_test 1 delete_blocks_test 2)
add_test(NAME shared_test4 COMMAND main_shared add_and_delete_test)

add_test(NAME dynamic_test1 COMMAND main_dynamic search_file_test 0 search_file_test 1 search_file_test 2)
add_test(NAME dynamic_test2 COMMAND main_dynamic save_blocks_test 0 save_blocks_test 1 save_blocks_test 2)
add_test(NAME dynamic_test3 COMMAND main_dynamic delete_blocks_test 0 delete_blocks_test 1 delete_blocks_test 2)
add_test(NAME dynamic_test4 COMMAND main_dynamic add_and_delete_test)

