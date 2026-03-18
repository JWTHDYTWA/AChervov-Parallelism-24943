add_library(fp_types INTERFACE fp_types.h)
# set_target_properties(fp_types PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features(fp_types INTERFACE cxx_std_20)
target_include_directories(fp_types INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(fp_types INTERFACE fp_types.h)
