cmake_minimum_required(VERSION 3.22)
project(raylib-test C CXX)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

add_subdirectory(raylib)

add_executable(raylib-test window.cpp)
target_compile_features(raylib-test PRIVATE cxx_std_20)
target_link_libraries(raylib-test PRIVATE raylib)
target_compile_definitions(raylib-test PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")