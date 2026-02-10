add_library(types INTERFACE types.h)
# set_target_properties(types PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features(types INTERFACE cxx_std_20)
target_include_directories(types INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(types INTERFACE types.h)
