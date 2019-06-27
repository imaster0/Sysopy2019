cmake_minimum_required(VERSION 2.8)

project(zad3b C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC ")

foreach(opt -O -O1 -O2 -O3 -Os)
    set(static_name "my_library_static${opt}")
    set(shared_name "my_library_shared${opt}")
    set(dynamic_name "my_library_dynamic${opt}")

    add_library (${static_name} STATIC ../zad1/library.c)
    add_library (${shared_name} SHARED ../zad1/library.c)
    add_library (${dynamic_name} MODULE ../zad1/library.c)

    target_compile_options(${static_name} PUBLIC ${opt})
    target_compile_options(${shared_name} PUBLIC ${opt})
    target_compile_options(${dynamic_name} PUBLIC ${opt})

    set(main_static "main_static${opt}")
    set(main_shared "main_shared${opt}")
    set(main_dynamic "main_dynamic${opt}")

    add_executable(${main_static} ../zad2/main.c)
    add_executable(${main_shared} ../zad2/main.c)
    add_executable(${main_dynamic} main_dynamic.c)

    target_compile_options(${main_static} PUBLIC ${opt})
    target_compile_options(${main_shared} PUBLIC ${opt})
    target_compile_options(${main_dynamic} PUBLIC ${opt})

    target_link_libraries(${main_static} ${static_name})
    target_link_libraries(${main_dynamic} ${CMAKE_DL_LIBS})
    target_link_libraries(${main_shared} ${shared_name})
endforeach(opt)

enable_testing()

foreach(opt -O -O1 -O2 -O3 -Os)
    set(main_static "main_static${opt}")
    set(main_shared "main_shared${opt}")
    set(main_dynamic "main_dynamic${opt}")
    set(dynamic_lib "./libmy_library_dynamic${opt}.so")

    set(static_test1  "static_test1${opt}")
    set(static_test2 "static_test2${opt}")
    set(static_test3 "static_test3${opt}")
    set(static_test4 "static_test4${opt}")

    set(shared_test1 "shared_test1${opt}")
    set(shared_test2 "shared_test2${opt}")
    set(shared_test3 "shared_test3${opt}")
    set(shared_test4 "shared_test4${opt}")

    set(dynamic_test1 "dynamic_test1${opt}")
    set(dynamic_test2 "dynamic_test2${opt}")
    set(dynamic_test3 "dynamic_test3${opt}")
    set(dynamic_test4 "dynamic_test4${opt}")

    add_test(NAME ${static_test1} COMMAND ${main_static} search_file_test 0 search_file_test 1 search_file_test 2)
    add_test(NAME ${static_test2} COMMAND ${main_static} save_blocks_test 0 save_blocks_test 1 save_blocks_test 2)
    add_test(NAME ${static_test3} COMMAND ${main_static} delete_blocks_test 0 delete_blocks_test 1 delete_blocks_test 2)
    add_test(NAME ${static_test4} COMMAND ${main_static} add_and_delete_test)

    add_test(NAME ${shared_test1} COMMAND ${main_shared} search_file_test 0 search_file_test 1 search_file_test 2)
    add_test(NAME ${shared_test2} COMMAND ${main_shared} save_blocks_test 0 save_blocks_test 1 save_blocks_test 2)
    add_test(NAME ${shared_test3} COMMAND ${main_shared} delete_blocks_test 0 delete_blocks_test 1 delete_blocks_test 2)
    add_test(NAME ${shared_test4} COMMAND ${main_shared} add_and_delete_test)

    add_test(NAME ${dynamic_test1} COMMAND ${main_dynamic} ${dynamic_lib} search_file_test 0 search_file_test 1 search_file_test 2)
    add_test(NAME ${dynamic_test2} COMMAND ${main_dynamic} ${dynamic_lib} save_blocks_test 0 save_blocks_test 1 save_blocks_test 2)
    add_test(NAME ${dynamic_test3} COMMAND ${main_dynamic} ${dynamic_lib} delete_blocks_test 0 delete_blocks_test 1 delete_blocks_test 2)
    add_test(NAME ${dynamic_test4} COMMAND ${main_dynamic} ${dynamic_lib} add_and_delete_test)


endforeach(opt)