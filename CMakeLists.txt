cmake_minimum_required(VERSION 3.22)
project(HandGestureUserStudy C CXX)

# ========================================
# ========== User configuration ==========
# ========================================

# targets
set(MAIN_EXECUTABLE_NAME gestureDriver)
set(TEST_VIRTUAL_MOUSE virtualMouse)
set(TEST_HTTP_SERVER httpServer)
set(TEST_THREADING threadingTest)
set(TEST_LOGGING loggingTest)

# include directories
set(INCLUDE_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/)
set(INCLUDE_CPP_HTTPLIB ${CMAKE_CURRENT_SOURCE_DIR}/cpp-httplib/)
set(INCLUDE_RAYLIB ${CMAKE_CURRENT_SOURCE_DIR}/raylib/src/)
set(INCLUDE_RAPIDJSON ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include/)
set(INCLUDE_LEAPSDK ${CMAKE_CURRENT_SOURCE_DIR}/LeapSDK/include/)

# =======================================================
# ========== Leap Motion library configuration ==========
# =======================================================
find_package(LeapSDK 5 PATHS ${CMAKE_CURRENT_SOURCE_DIR}/LeapSDK/lib/cmake/LeapSDK)

add_custom_target(libLeapC)
add_custom_command(
    TARGET libLeapC
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/LeapSDK/lib/x64/LeapC.dll
            ${CMAKE_BINARY_DIR}/$<CONFIG>
    BYPRODUCTS ${CMAKE_BINARY_DIR}/$<CONFIG>/LeapC.dll)

# =======================================================
# =============== Static file server fles ===============
# =======================================================
add_custom_target(staticFiles)
add_custom_command(
    TARGET staticFiles
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/www
            ${CMAKE_BINARY_DIR}/$<CONFIG>/www
    BYPRODUCTS ${CMAKE_BINARY_DIR}/$<CONFIG>/www/
)

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
    Input/LeapConnection.cpp
    Input/LeapMotionGestureProvider.cpp
    Input/SimulatedMouse.cpp
    Math/MathHelpers.cpp
    Math/Vector3Common.cpp
    Programs/Old/GestureDriver.cpp
    Visualization/RaylibVisuals.cpp)

add_dependencies(${MAIN_EXECUTABLE_NAME} libLeapC)

target_compile_features(${MAIN_EXECUTABLE_NAME} PRIVATE cxx_std_20)

target_include_directories(
    ${MAIN_EXECUTABLE_NAME} PRIVATE ${INCLUDE_MAIN} ${INCLUDE_RAYLIB} ${INCLUDE_CPP_HTTPLIB}
                                    ${INCLUDE_RAPIDJSON} ${INCLUDE_LEAPSDK})

target_link_libraries(${MAIN_EXECUTABLE_NAME} PRIVATE raylib LeapSDK::LeapC)

# ASSETS_PATH => required for raylib assets (unused)
# I am also using _DEBUG, which is an MSVC
# exclusive symbol that is defined implicitly in debug builds.
target_compile_definitions(${MAIN_EXECUTABLE_NAME}
                           PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

# ============================================================
# ========== Virtual mouse input test configuration ==========
# ============================================================

add_executable(${TEST_VIRTUAL_MOUSE} Input/SimulatedMouse.cpp Programs/Testing/VirtualMouseTest.cpp)

add_dependencies(${TEST_VIRTUAL_MOUSE} libLeapC)

target_compile_features(${TEST_VIRTUAL_MOUSE} PRIVATE cxx_std_20)

target_include_directories(${TEST_VIRTUAL_MOUSE} PRIVATE ${INCLUDE_MAIN})

target_link_libraries(${TEST_VIRTUAL_MOUSE} PRIVATE LeapSDK::LeapC)

# ============================================================
# ============== HTTP server test configuration ==============
# ============================================================

add_executable(${TEST_HTTP_SERVER} Programs/Testing/CppHttplibTest.cpp)

target_include_directories(${TEST_HTTP_SERVER} PRIVATE ${INCLUDE_MAIN} ${INCLUDE_CPP_HTTPLIB}
                                                       ${INCLUDE_RAPIDJSON})

target_compile_features(${TEST_HTTP_SERVER} PRIVATE cxx_std_20)

# ============================================================
# =============== Threading test configuration ===============
# ============================================================

add_executable(
    ${TEST_THREADING}
    Input/LeapConnection.cpp
    Input/LeapMotionGestureProvider.cpp
    Input/SimulatedMouse.cpp
    Math/MathHelpers.cpp
    Math/Vector3Common.cpp
    Programs/UserStudy/HttpServer.cpp
    Programs/UserStudy/LeapDriver.cpp
    Programs/UserStudy/ThreadingTest.cpp
    Programs/UserStudy/Visualizer.cpp
    Visualization/RaylibVisuals.cpp
    Helpers/UserIDLock.cpp)

add_dependencies(${TEST_THREADING} libLeapC staticFiles)

target_include_directories(
    ${TEST_THREADING} PRIVATE ${INCLUDE_MAIN} ${INCLUDE_CPP_HTTPLIB} ${INCLUDE_RAYLIB}
                              ${INCLUDE_LEAPSDK} ${INCLUDE_RAPIDJSON})

target_compile_features(${TEST_THREADING} PRIVATE cxx_std_20)

target_link_libraries(${TEST_THREADING} PRIVATE raylib LeapSDK::LeapC)

# ============================================================
# ================ Logging test configuration ================
# ============================================================

add_executable(${TEST_LOGGING} Programs/Testing/LoggerTest.cpp Programs/UserStudy/Logging.cpp)

target_compile_features(${TEST_LOGGING} PRIVATE cxx_std_20)
