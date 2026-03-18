add_library(matrix INTERFACE)

target_compile_features(matrix INTERFACE cxx_std_20)
target_include_directories(matrix INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
