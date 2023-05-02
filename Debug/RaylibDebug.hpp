#pragma once

#define Font __RAYLIB_FONT_T
#include "../raylib/src/raylib.h"
#undef Font
#include "../raylib/src/rcamera.h"

// Undefine numerical constants with similar names to constants in the LeapSDK.
// PI in particular has a breaking name collision with Leap::PI.
// (Doesn't math.h define M_PI with sufficient precision anyway?)
#undef PI
#undef DEG2RAD
#undef RAD2DEG

#include "../Helpers.hpp"

namespace Debug
{
constexpr int SCREEN_WIDTH = 1600;
constexpr int SCREEN_HEIGHT = 900;

constexpr float CAMERA_MOVE_SPEED = 0.09f;
constexpr float CAMERA_ROTATION_SPEED = 0.03f;

constexpr float VECTOR_BASIS_LENGTH = 10.0f;

constexpr Vector3 VECTOR_ORIGIN = {0.0f, 0.0f, 0.0f};
constexpr Vector3 VECTOR_BASIS_I = {VECTOR_BASIS_LENGTH, 0.0f, 0.0f};
constexpr Vector3 VECTOR_BASIS_J = {0.0f, VECTOR_BASIS_LENGTH, 0.0f};
constexpr Vector3 VECTOR_BASIS_K = {0.0f, 0.0f, VECTOR_BASIS_LENGTH};

constexpr Color VECTOR_BASIS_I_COLOR = RED;
constexpr Color VECTOR_BASIS_J_COLOR = GREEN;
constexpr Color VECTOR_BASIS_K_COLOR = BLUE;

inline void DrawCartesianBasis()
{
    DrawLine3D(VECTOR_ORIGIN, VECTOR_BASIS_I, VECTOR_BASIS_I_COLOR);
    DrawLine3D(VECTOR_ORIGIN, VECTOR_BASIS_J, VECTOR_BASIS_J_COLOR);
    DrawLine3D(VECTOR_ORIGIN, VECTOR_BASIS_K, VECTOR_BASIS_K_COLOR);
}

inline void DrawVectorDecomposition(Vector3 vec, bool shouldSumVectors)
{
    if (shouldSumVectors)
    {
        DrawLine3D(VECTOR_ORIGIN, Vector3{vec.x, 0.0f, 0.0f}, VECTOR_BASIS_I_COLOR);
        DrawLine3D(Vector3{vec.x, 0.0f, 0.0f}, Vector3{vec.x, 0.0f, vec.z}, VECTOR_BASIS_K_COLOR);
        DrawLine3D(Vector3{vec.x, 0.0f, vec.z}, vec, VECTOR_BASIS_J_COLOR);
    }
    else
    {
        DrawLine3D(VECTOR_ORIGIN, Vector3{vec.x, 0.0f, 0.0f}, VECTOR_BASIS_I_COLOR);
        DrawLine3D(VECTOR_ORIGIN, Vector3{0.0f, 0.0f, vec.z}, VECTOR_BASIS_K_COLOR);
        DrawLine3D(VECTOR_ORIGIN, Vector3{0.0f, vec.y, 0.0f}, VECTOR_BASIS_J_COLOR);
    }
}

inline void UpdateCamera(Camera& camera)
{
    if (IsKeyDown(KEY_DOWN)) CameraPitch(&camera, -CAMERA_ROTATION_SPEED, true, false, false);
    if (IsKeyDown(KEY_UP)) CameraPitch(&camera, CAMERA_ROTATION_SPEED, true, false, false);
    if (IsKeyDown(KEY_RIGHT)) CameraYaw(&camera, -CAMERA_ROTATION_SPEED, false);
    if (IsKeyDown(KEY_LEFT)) CameraYaw(&camera, CAMERA_ROTATION_SPEED, false);

    if (IsKeyDown(KEY_W)) CameraMoveForward(&camera, CAMERA_MOVE_SPEED, true);
    if (IsKeyDown(KEY_A)) CameraMoveRight(&camera, -CAMERA_MOVE_SPEED, true);
    if (IsKeyDown(KEY_S)) CameraMoveForward(&camera, -CAMERA_MOVE_SPEED, true);
    if (IsKeyDown(KEY_D)) CameraMoveRight(&camera, CAMERA_MOVE_SPEED, true);
    if (IsKeyDown(KEY_SPACE)) CameraMoveUp(&camera, CAMERA_MOVE_SPEED);
    if (IsKeyDown(KEY_LEFT_SHIFT)) CameraMoveUp(&camera, -CAMERA_MOVE_SPEED);

    if (IsKeyDown(KEY_KP_1))
    {
        camera.target = Vector3{0.0f, 0.0f, 0.0f};
        camera.up = Vector3{0.0f, 1.0f, 0.0f};
        camera.position = Vector3{10.0f, 0.0f, 0.0f};
        std::cout << "Looking at YZ plane (pos=(10,0,0))" << std::endl;
    }

    if (IsKeyDown(KEY_KP_2))
    {
        camera.target = Vector3{0.0f, 0.0f, 0.0f};
        camera.up = Vector3{0.0f, 1.0f, 0.0f};
        camera.position = Vector3{0.0f, 0.0f, 10.0f};
        std::cout << "Looking at XY plane (pos=(0,0,10))" << std::endl;
    }

    if (IsKeyDown(KEY_KP_3))
    {
        camera.target = Vector3{0.0f, 0.0f, 0.0f};
        camera.up = Vector3{0.0f, 1.0f, 0.0f};
        camera.position = Vector3{1.0f, 10.0f, 1.0f};
        std::cout << "Looking at XZ plane from above (pos=(1,10,1))" << std::endl;
    }

    if (IsKeyDown(KEY_KP_4))
    {
        camera.target = Vector3{0.0f, 0.0f, 0.0f};
        camera.up = Vector3{0.0f, 1.0f, 0.0f};
        camera.position = Vector3{1.0f, -10.0f, 1.0f};
        std::cout << "Looking at XZ plane from below (pos=(1,-10,1))" << std::endl;
    }
}

inline void DrawCone(const Vector3 coneAxis, const Vector3 circleRotationAxis,
                     const float coneAngle, const float coneHeight, const Color coneColor)
{
    const float coneBaseRadius = coneHeight * std::tan(coneAngle);
    // const Vector3 heightVector = Helpers::ScaleRaylibVec3(coneAxis, coneHeight);
    DrawCircle3D(Helpers::ScaleRaylibVec3(coneAxis, 0.25f * coneHeight), 0.25f * coneBaseRadius,
                 circleRotationAxis, 90.0f, coneColor);
    DrawCircle3D(Helpers::ScaleRaylibVec3(coneAxis, 0.50f * coneHeight), 0.50f * coneBaseRadius,
                 circleRotationAxis, 90.0f, coneColor);
    DrawCircle3D(Helpers::ScaleRaylibVec3(coneAxis, 0.75f * coneHeight), 0.75f * coneBaseRadius,
                 circleRotationAxis, 90.0f, coneColor);
    DrawCircle3D(Helpers::ScaleRaylibVec3(coneAxis, 1.00f * coneHeight), 1.00f * coneBaseRadius,
                 circleRotationAxis, 90.0f, coneColor);

    const float hypotenuseLength = coneHeight / std::cos(coneAngle);

    // Vector3 bound1 = Vector3RotateByAxisAngle(coneAngle, )
}

}  // namespace Debug