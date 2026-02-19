cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(CircleSin VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_DOUBLE "Use double type instead of float" OFF)
option(EN_PARALLELISM "Enable parallelism" OFF)

if(USE_DOUBLE)
    add_compile_definitions(USE_DOUBLE)
endif()

find_package(OpenMP REQUIRED)
add_subdirectory(inc)

add_executable(circlesin main.cpp)
target_link_libraries(circlesin PUBLIC types)

if(EN_PARALLELISM)
    target_link_libraries(circlesin PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(circlesin PRIVATE EN_PARALLELISM)
endif(EN_PARALLELISM)
