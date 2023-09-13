cmake_minimum_required(VERSION 3.22)
project(HandGestureUserStudy C CXX)

# ========================================
# ========== User configuration ==========
# ========================================

# targets
set(MAIN_EXECUTABLE_NAME handGestureUserStudy)
set(OLD_EXECUTABLE_NAME gestureDriver)
set(TEST_VIRTUAL_MOUSE virtualMouse)
set(TEST_HTTP_SERVER httpServer)
set(TEST_LOGGING loggingTest)
set(TEST_TEMPLATING templatingTest)
set(TEST_SSE sseReliabilityTest)

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
# ================= Static file copying =================
# =======================================================
add_custom_target(staticFiles)
add_custom_command(
    TARGET staticFiles
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/www
            ${CMAKE_BINARY_DIR}/$<CONFIG>/www
    BYPRODUCTS ${CMAKE_BINARY_DIR}/$<CONFIG>/www)

add_custom_target(htmlTemplates)
add_custom_command(
    TARGET htmlTemplates
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/HTMLTemplates
            ${CMAKE_BINARY_DIR}/$<CONFIG>/HTMLTemplates
    BYPRODUCTS ${CMAKE_BINARY_DIR}/$<CONFIG>/HTMLTemplates
)

add_custom_target(3dModels)
add_custom_command(
    TARGET 3dModels
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Models
            ${CMAKE_BINARY_DIR}/$<CONFIG>/Models
    BYPRODUCTS ${CMAKE_BINARY_DIR}/$<CONFIG>/Models
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
# ======== Old main executable configuration ========
# ===================================================

add_executable(
    ${OLD_EXECUTABLE_NAME}
    Input/LeapConnection.cpp
    Input/LeapMotionGestureProvider.cpp
    Input/SimulatedMouse.cpp
    Math/MathHelpers.cpp
    Math/Vector3Common.cpp
    Programs/Old/GestureDriver.cpp
    Visualization/RaylibVisuals.cpp)

add_dependencies(${OLD_EXECUTABLE_NAME} libLeapC)

target_compile_features(${OLD_EXECUTABLE_NAME} PRIVATE cxx_std_20)

target_include_directories(
    ${OLD_EXECUTABLE_NAME} PRIVATE ${INCLUDE_MAIN} ${INCLUDE_RAYLIB} ${INCLUDE_CPP_HTTPLIB}
                                    ${INCLUDE_RAPIDJSON} ${INCLUDE_LEAPSDK})

target_link_libraries(${OLD_EXECUTABLE_NAME} PRIVATE raylib LeapSDK::LeapC)

# ASSETS_PATH => required for raylib assets (unused)
# I am also using _DEBUG, which is an MSVC
# exclusive symbol that is defined implicitly in debug builds.
target_compile_definitions(${OLD_EXECUTABLE_NAME}
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
# ================= User study configuration =================
# ============================================================

add_executable(
    ${MAIN_EXECUTABLE_NAME}
    Helpers/UserIDLock.cpp
    Helpers/JSONEvents.cpp
    HTML/HTMLTemplate.cpp
    Input/LeapConnection.cpp
    Input/LeapMotionGestureProvider.cpp
    Input/SimulatedMouse.cpp
    Math/MathHelpers.cpp
    Math/Vector3Common.cpp
    Programs/UserStudy/HttpServer.cpp
    Programs/UserStudy/LeapDriver.cpp
    Programs/UserStudy/Main.cpp
    Programs/UserStudy/Visualizer.cpp
    Programs/UserStudy/Logging.cpp
    Programs/UserStudy/CursorLogger.cpp
    Visualization/RaylibVisuals.cpp)

add_dependencies(${MAIN_EXECUTABLE_NAME} libLeapC staticFiles htmlTemplates 3dModels)

target_include_directories(
    ${MAIN_EXECUTABLE_NAME} PRIVATE ${INCLUDE_MAIN} ${INCLUDE_CPP_HTTPLIB} ${INCLUDE_RAYLIB}
                              ${INCLUDE_LEAPSDK} ${INCLUDE_RAPIDJSON})

target_compile_features(${MAIN_EXECUTABLE_NAME} PRIVATE cxx_std_20)

target_link_libraries(${MAIN_EXECUTABLE_NAME} PRIVATE raylib LeapSDK::LeapC)

# ============================================================
# ================ Logging test configuration ================
# ============================================================

add_executable(${TEST_LOGGING} Programs/Testing/LoggerTest.cpp Programs/UserStudy/Logging.cpp)

target_compile_features(${TEST_LOGGING} PRIVATE cxx_std_20)

# ============================================================
# ============ HTML templating test configuration ============
# ============================================================

add_executable(${TEST_TEMPLATING} HTML/HTMLTemplate.cpp
                                  Programs/Testing/TemplatingTest.cpp)
add_dependencies(${TEST_TEMPLATING} staticFiles htmlTemplates)
target_include_directories(${TEST_TEMPLATING} PRIVATE ${INCLUDE_MAIN} ${INCLUDE_CPP_HTTPLIB})
target_compile_features(${TEST_TEMPLATING} PRIVATE cxx_std_20)

# ============================================================
# ============ SSE reliability test configuration ============
# ============================================================

add_executable(${TEST_SSE} Programs/Testing/SSEReliabilityTest.cpp)
add_dependencies(${TEST_SSE} staticFiles)
target_include_directories(${TEST_SSE} PRIVATE ${INCLUDE_MAIN} ${INCLUDE_CPP_HTTPLIB})
target_compile_features(${TEST_SSE} PRIVATE cxx_std_20)
