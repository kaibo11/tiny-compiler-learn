cmake_minimum_required(VERSION 3.10)


project(MyTest)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

enable_testing()

add_subdirectory(googletest)

add_subdirectory(JsonCpp)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})


file(GLOB PARSER_SOURCES "${CMAKE_CURRENT_LIST_DIR}/parser/*.cpp")

add_executable(MyTest test.cpp ${PARSER_SOURCES})

target_link_libraries(MyTest PRIVATE nlohmann_json::nlohmann_json gtest_main)

add_test(NAME MyTest COMMAND MyTest)
