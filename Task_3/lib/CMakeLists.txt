add_library(libs INTERFACE)

target_compile_features(libs INTERFACE cxx_std_20)
target_include_directories(libs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
