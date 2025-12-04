project(binghamton)

option(BINGHAMTON_BUILD_TEST "Builds gtest executables for all methods" ON)

file(GLOB_RECURSE binghamton_source "source/*.cpp")
add_library(binghamton STATIC ${binghamton_source})
set_property(TARGET binghamton PROPERTY CXX_STANDARD 17)
target_include_directories(binghamton PUBLIC "include")

if(BINGHAMTON_BUILD_TEST)
    set(gtest_force_shared_crt ON)
    add_subdirectory("external/gtest")
    add_subdirectory("external/stb")
    target_sources(binghamton PUBLIC "test/gtest_env.cpp")
    target_link_libraries(binghamton PUBLIC GTest::gtest_main stb)

    # WOW
    add_executable(binghamton_gtest_wow "test/gtest_wow.cpp")
    set_target_properties(binghamton_gtest_wow PROPERTIES CXX_STANDARD 17)
    target_link_libraries(binghamton_gtest_wow PRIVATE binghamton)
endif()