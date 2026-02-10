cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(CircleSin VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_DOUBLE "Use double type instead of float" OFF)

if(USE_DOUBLE)
    add_compile_definitions(FPLEN=8)
else()
    add_compile_definitions(FPLEN=4)
endif()

find_package(OpenMP REQUIRED)
add_subdirectory(inc)

add_executable(circlesin main.cpp)
target_link_libraries(circlesin PUBLIC types OpenMP::OpenMP_CXX)
