cmake_minimum_required(VERSION 3.22)
project(HandGestureUserStudy C CXX)

# ========================================
# ========== User configuration ==========
# ========================================
set(MAIN_EXECUTABLE_NAME gestureDriver)
set(TEST_VIRTUAL_MOUSE virtualMouse)

# ======================================
# ========== Functions/macros ==========
# ======================================
macro(copy_file_to_output_directory target filename)
    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${filename} $<TARGET_FILE_DIR:${target}>)
endmacro()

# =======================================================
# ========== Leap Motion library configuration ==========
# =======================================================
find_package(LeapSDK 5 PATHS ${CMAKE_CURRENT_SOURCE_DIR}/LeapSDK/lib/cmake/LeapSDK)

# ==================================================
# ========== raylib library configuration ==========
# ==================================================
# Disables building the raylib examples and games
set(BUILD_EXAMPLES
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_GAMES
    OFF
    CACHE BOOL "" FORCE)

add_subdirectory(raylib)

# ===================================================
# ========== Main executable configuration ==========
# ===================================================
add_executable(
    ${MAIN_EXECUTABLE_NAME}
    Debug/LeapDebug.cpp
    Debug/RaylibDebug.cpp
    Helpers/LeapConnection.cpp
    Helpers/Vector3Common.cpp
    Input/LeapMotionGestureProvider.cpp
    Input/SimulatedMouse.cpp
    window.cpp)

copy_file_to_output_directory(${MAIN_EXECUTABLE_NAME}
                              ${CMAKE_CURRENT_SOURCE_DIR}/LeapSDK/lib/x64/LeapC.dll)

target_compile_features(${MAIN_EXECUTABLE_NAME} PRIVATE cxx_std_20)

target_link_libraries(${MAIN_EXECUTABLE_NAME} PRIVATE raylib LeapSDK::LeapC)

# ASSETS_PATH => required for raylib assets (unused)
# I am also using _DEBUG, which is an MSVC
# exclusive symbol that is defined implicitly in debug builds.
target_compile_definitions(${MAIN_EXECUTABLE_NAME}
                           PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

# ============================================================
# ========== Virtual mouse input test configuration ==========
# ============================================================

add_executable(${TEST_VIRTUAL_MOUSE} Input/SimulatedMouse.cpp Tests/VirtualMouseTest.cpp)

copy_file_to_output_directory(${TEST_VIRTUAL_MOUSE}
                              ${CMAKE_CURRENT_SOURCE_DIR}/LeapSDK/lib/x64/LeapC.dll)

target_compile_features(${TEST_VIRTUAL_MOUSE} PRIVATE cxx_std_20)

target_link_libraries(${TEST_VIRTUAL_MOUSE} PRIVATE LeapSDK::LeapC)
