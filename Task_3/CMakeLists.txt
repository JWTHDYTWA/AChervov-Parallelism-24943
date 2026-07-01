cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(Threads VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)
add_executable(matrix_mult_t matrix.cpp)
add_executable(client_server server.cpp)


find_package(OpenMP REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

target_link_libraries(matrix_mult_t PRIVATE libs Boost::program_options)
target_link_libraries(client_server PRIVATE libs Boost::program_options)
