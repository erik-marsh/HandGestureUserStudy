cmake_minimum_required(VERSION 3.22)
project(gesture-driver C CXX)

find_package(LeapSDK 5 PATHS ${CMAKE_CURRENT_SOURCE_DIR}/LeapSDK/lib/cmake/LeapSDK)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

add_subdirectory(raylib)

add_executable(gesture-driver
    window.cpp
    Input/SimulatedMouse.cpp
    Input/LeapMotionGestureProvider.cpp
    Debug/LeapDebug.cpp
    Helpers/Vector3Common.cpp
    Helpers/LeapConnection.cpp
)

add_custom_command(TARGET gesture-driver
	POST_BUILD
	COMMAND
		${CMAKE_COMMAND} -E copy
		${CMAKE_CURRENT_SOURCE_DIR}/LeapSDK/lib/x64/LeapC.dll
		$<TARGET_FILE_DIR:gesture-driver>
)

target_compile_features(gesture-driver PRIVATE
    cxx_std_20
)

target_link_libraries(gesture-driver PRIVATE
    raylib
    LeapSDK::LeapC
)

target_compile_definitions(gesture-driver PUBLIC
    ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
)
