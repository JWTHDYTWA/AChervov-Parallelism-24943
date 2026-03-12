cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(MatrixMult VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)

add_executable(matrix_mult main.cpp)
