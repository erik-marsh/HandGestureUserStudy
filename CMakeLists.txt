cmake_minimum_required(VERSION 3.22)
project(gesture-driver C CXX)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

add_subdirectory(raylib)

add_executable(gesture-driver
    window.cpp
    Input/SimulatedMouse.cpp
    xdotool/xdo.c
    Input/LeapMotionGestureProvider.cpp
    Debug/LeapDebug.cpp
    # Debug/RaylibDebug.cpp
)
add_compile_options(gesture-driver 
    -Wl,-rpath,${PROJECT_SOURCE_DIR}/LeapSDK/lib/x64
)
target_compile_features(gesture-driver PRIVATE
    cxx_std_20
)
target_link_libraries(gesture-driver PRIVATE
    raylib
    ${PROJECT_SOURCE_DIR}/LeapSDK/lib/x64/libLeap.so
    X11
    Xtst
    Xinerama
    xkbcommon
    Xi
)
target_compile_definitions(gesture-driver PUBLIC
    ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
)
