cmake_minimum_required(VERSION 3.14)
project(MemDbDriver)


# Set compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -pedantic-errors -g -O3")

#add_subdirectory(lib/memdb)

include_directories(include)
include_directories(lib/memdb/include)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_executable(driver src/driver.cpp)
add_executable(test src/test.cpp)
