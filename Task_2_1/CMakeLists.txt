cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(MatrixMult VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)
find_package(OpenMP REQUIRED)

add_executable(matrix_mult main.cpp)
target_link_libraries(matrix_mult PRIVATE matrix OpenMP::OpenMP_CXX)
