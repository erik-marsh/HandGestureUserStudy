#include "RaylibDebug.hpp"

#include <cmath>
#include <iostream>

#include "../Helpers/Vector3Common.hpp"

namespace Debug
{

void DrawCartesianBasis()
{
    DrawLine3D(VECTOR_ORIGIN, VECTOR_BASIS_I, VECTOR_BASIS_I_COLOR);
    DrawLine3D(VECTOR_ORIGIN, VECTOR_BASIS_J, VECTOR_BASIS_J_COLOR);
    DrawLine3D(VECTOR_ORIGIN, VECTOR_BASIS_K, VECTOR_BASIS_K_COLOR);
}

void DrawVectorDecomposition(Vector3 vec, bool shouldSumVectors)
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

void UpdateCamera(Camera& camera)
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

void DrawCone(const Vector3 coneAxis, const Vector3 circleRotationAxis, const float coneAngle,
              const float coneHeight, const Color coneColor)
{
    const float coneBaseRadius = coneHeight * std::tan(coneAngle);

    for (int i = 1; i <= 4; i++)
    {
        float scalar = 0.25f * i;
        Vector3 scaledAxis = Vector3Scale(coneAxis, scalar * coneHeight);
        DrawCircle3D(scaledAxis, scalar * coneBaseRadius, circleRotationAxis, 90.0f, coneColor);
    }
}

void DrawHand(LEAP_HAND& hand)
{
    Helpers::Vector3Common armEnd(hand.arm.next_joint);

    // Draw all of the bones of the index, middle, ring, and pinky fingers.
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            Helpers::Vector3Common boneStart(hand.digits[i].bones[j].prev_joint);
            Helpers::Vector3Common boneEnd(hand.digits[i].bones[j].next_joint);

            boneStart = Helpers::Vector3Common::Subtract(boneStart, armEnd);
            boneEnd = Helpers::Vector3Common::Subtract(boneEnd, armEnd);

            constexpr float scaleFactor = 0.01f;
            boneStart = Helpers::Vector3Common::ScalarMultiply(boneStart, scaleFactor);
            boneEnd = Helpers::Vector3Common::ScalarMultiply(boneEnd, scaleFactor);

            // Since we do calculations on the distal bone, we highlight it.
            Color color = j == 3 ? RED : BLACK;
            DrawLine3D(boneStart.AsRaylib(), boneEnd.AsRaylib(), color);
        }
    }
}

}  // namespace Debug