project(stb)

file(GLOB_RECURSE stb_source "source/*.cpp")
add_library(stb STATIC ${stb_source})
set_property(TARGET stb PROPERTY CXX_STANDARD 17)
target_include_directories(stb PUBLIC "include")

if(MSVC)
    target_compile_definitions(stb PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()
