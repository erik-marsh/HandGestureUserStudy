#include <cmath>
#include <iostream>

#include "raylib/src/raylib.h"

int main()
{
    std::cout << "test" << std::endl;
    constexpr int screenWidth = 800;
    constexpr int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "something something idk");

    // Camera definitions
    Camera3D camera{};
    camera.position = (Vector3){10.0f, 10.0f, 10.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Vector3 cubePosition = {0.0f, 0.0f, 0.0f};

    DisableCursor();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_THIRD_PERSON);

        if (IsKeyDown('Z')) camera.target = (Vector3){0.0f, 0.0f, 0.0f};

        BeginDrawing();
        {
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
            {
                double currTime = GetTime();

                Vector3 start = {0.0f, 0.0f, 0.0f};
                Vector3 end = {1.0 * std::sin(currTime), 1.0f * std::cos(currTime), 0.0f};
                DrawLine3D(start, end, RED);
            }
            EndMode3D();
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}