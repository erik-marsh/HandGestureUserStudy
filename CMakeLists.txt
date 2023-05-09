cmake_minimum_required(VERSION 3.22)
project(gesture-driver C CXX)

find_package(LeapSDK 5 PATHS ${CMAKE_CURRENT_SOURCE_DIR}/LeapSDK/lib/cmake/LeapSDK)

# Disables building the raylib examples and games
set(BUILD_EXAMPLES
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_GAMES
    OFF
    CACHE BOOL "" FORCE)

add_subdirectory(raylib)

add_executable(
    gesture-driver window.cpp Input/SimulatedMouse.cpp Input/LeapMotionGestureProvider.cpp
                   Debug/LeapDebug.cpp Helpers/Vector3Common.cpp Helpers/LeapConnection.cpp)

# The Leap Motion SDK requires a .dll, so we copy it from the LeapSDK dir to the output dir.
add_custom_command(
    TARGET gesture-driver
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/LeapSDK/lib/x64/LeapC.dll
            $<TARGET_FILE_DIR:gesture-driver>)

target_compile_features(gesture-driver PRIVATE cxx_std_20)

target_link_libraries(gesture-driver PRIVATE raylib LeapSDK::LeapC)

# ASSETS_PATH => required for raylib assets (unused) I am also using _DEBUG, which is an MSVC
# exclusive symbol that is defined implicitly in debug builds.
target_compile_definitions(gesture-driver PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")
