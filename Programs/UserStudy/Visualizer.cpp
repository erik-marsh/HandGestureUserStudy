#include "Visualizer.hpp"

#include <raylib.h>
#include <rcamera.h>

#include <Visualization/RaylibVisuals.hpp>
#include <iostream>

namespace Visualization
{

void RenderLoop(Renderables& renderables, std::atomic<bool>& isRunning)
{
    std::cout << "Starting rendering thread..." << std::endl;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Gesture Driver Debug Visualizer");
    SetTargetFPS(60);

    Camera3D camera3D{};
    camera3D.position = Vector3{10.0f, 10.0f, 10.0f};
    camera3D.target = Vector3{0.0f, 0.0f, 0.0f};
    camera3D.up = Vector3{0.0f, 1.0f, 0.0f};
    camera3D.fovy = 45.0f;
    camera3D.projection = CAMERA_PERSPECTIVE;

    Camera2D camera2D{};
    camera2D.zoom = 1.0f;

    constexpr int rectCenterX = 100;
    constexpr int rectCenterY = SCREEN_HEIGHT - 100;
    constexpr int rectWidth = 200;
    constexpr int rectHeight = 200;
    constexpr int vectorLength = 100;

    while (isRunning.load())
    {
        UpdateCamera(camera3D);

        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        BeginMode3D(camera3D);
        DrawCartesianBasis();
        for (int i = 0; i < renderables.hands.size(); i++)
            DrawHand(renderables.hands[i]);
        EndMode3D();

        BeginMode2D(camera2D);
        const int cursorDirectionVecX =
            static_cast<int>(renderables.cursorDirectionX * vectorLength);
        // multiply by -1 because Y is down in screen space
        const int cursorDirectionVecY =
            static_cast<int>(-1.0f * renderables.cursorDirectionY * vectorLength);

        const int fingerDirectionVecX =
            static_cast<int>(renderables.averageFingerDirectionX * vectorLength * 0.5f);
        const int fingerDirectionVecY =
            static_cast<int>(-1.0f * renderables.averageFingerDirectionY * vectorLength * 0.5f);

        DrawRectangle(rectCenterX - (rectWidth / 2), rectCenterY - (rectHeight / 2),
                      rectCenterX + (rectWidth / 2), rectCenterY + (rectHeight / 2), DARKGRAY);

        DrawLine(rectCenterX, rectCenterY, rectCenterX + cursorDirectionVecX,
                 rectCenterY + cursorDirectionVecY, YELLOW);
        DrawLine(rectCenterX, rectCenterY, rectCenterX + fingerDirectionVecX,
                 rectCenterY + fingerDirectionVecY, PURPLE);
        EndMode2D();

        DrawText(renderables.leapDebugString.c_str(), 10, 10, 10, BLACK);
        EndDrawing();
    }

    CloseWindow();
    std::cout << "Shutting down rendering thread..." << std::endl;
}

}  // namespace Visualization