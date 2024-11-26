cmake_minimum_required(VERSION 3.14)
project(Memdb)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -pedantic-errors -g")

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(./include)

enable_testing()

add_executable(
    memdb_test  test/memdb_test.cpp
)

add_executable(  
    lexer_test  test/lexer_test.cpp
)

target_link_libraries(
    memdb_test    
    GTest::gtest_main
)

target_link_libraries(
    lexer_test    
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(memdb_test)
gtest_discover_tests(lexer_test)