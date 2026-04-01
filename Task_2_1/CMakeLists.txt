cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(MatrixMult VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_TOOLCHAIN_FILE "%VCPKG_ROOT%\\scripts\\buildsystems\\vcpkg.cmake")

add_subdirectory(lib)
find_package(OpenMP REQUIRED)
# find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(matrix_mult main.cpp)
target_link_libraries(matrix_mult PRIVATE matrix OpenMP::OpenMP_CXX)
