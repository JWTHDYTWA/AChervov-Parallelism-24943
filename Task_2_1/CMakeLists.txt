cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(MatrixMult VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)
add_executable(matrix_mult main.cpp)

if(MSVC)
    target_compile_options(matrix_mult PRIVATE -openmp:llvm)
else()
    find_package(OpenMP REQUIRED)
    target_link_libraries(matrix_mult PRIVATE OpenMP::OpenMP_CXX)
endif()
find_package(Boost REQUIRED COMPONENTS program_options)

target_link_libraries(matrix_mult PRIVATE matrix Boost::program_options)
