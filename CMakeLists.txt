project(binghamton)

option(BINGHAMTON_BUILD_TEST "Builds a gtest executable" ON)

file(GLOB_RECURSE binghamton_source "source/*.cpp")
add_library(binghamton STATIC ${binghamton_source})
set_property(TARGET binghamton PROPERTY CXX_STANDARD 17)
target_include_directories(binghamton PUBLIC "include")

if(BINGHAMTON_BUILD_TEST)
    set(gtest_force_shared_crt ON)
    add_subdirectory("external/gtest")
    add_subdirectory("external/stb")
    file(GLOB_RECURSE binghamton_gtest_source "test/*.cpp")
    add_executable(binghamton_gtest ${binghamton_gtest_source})
    set_target_properties(binghamton_gtest PROPERTIES CXX_STANDARD 17)
    target_link_libraries(binghamton_gtest PRIVATE binghamton GTest::gtest_main stb)
endif()