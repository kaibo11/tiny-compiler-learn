cmake_minimum_required(VERSION 3.10)


project(WasmParser)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

set(CMAKE_CXX_CLANG_TIDY clang-tidy --config-file ${CMAKE_CURRENT_LIST_DIR}/../.clang-tidy "--header-filter=${CMAKE_CURRENT_LIST_DIR}/(src|tests)/.*")

add_executable(WasmParser main.cpp)
