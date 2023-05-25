#include "Visualizer.hpp"

#define _____
#include <raylib.h>
#define ____
#include <rcamera.h>
#define ___
#include <Visualization/RaylibVisuals.hpp>

namespace Visualization
{

void RenderLoop(Renderables& renderables, std::atomic<bool>& isRendering)
{
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

    while (isRendering.load())
    {
        UpdateCamera(camera3D);

        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        BeginMode3D(camera3D);
        DrawCartesianBasis();
        for (int i = 0; i < renderables.hands.size(); i++) DrawHand(renderables.hands[i]);
        EndMode3D();

        BeginMode2D(camera2D);
        constexpr int rectCenterX = 100;
        constexpr int rectCenterY = SCREEN_HEIGHT - 100;

        constexpr int rectWidth = 200;
        constexpr int rectHeight = 200;
        constexpr int vectorLength = 100;

        const int vectorX = static_cast<int>(renderables.cursorDirectionX * vectorLength);
        const int vectorY = static_cast<int>(-1.0f * renderables.cursorDirectionY *
                                             vectorLength);  // -1 because Y is down in screen space

        const int debugVectorX =
            static_cast<int>(renderables.averageFingerDirectionX * vectorLength * 0.5f);
        const int debugVectorY =
            static_cast<int>(-1.0f * renderables.averageFingerDirectionY * vectorLength * 0.5f);

        DrawRectangle(rectCenterX - (rectWidth / 2), rectCenterY - (rectHeight / 2),
                      rectCenterX + (rectWidth / 2), rectCenterY + (rectHeight / 2), DARKGRAY);

        DrawLine(rectCenterX, rectCenterY, rectCenterX + vectorX, rectCenterY + vectorY, YELLOW);
        DrawLine(rectCenterX, rectCenterY, rectCenterX + debugVectorX, rectCenterY + debugVectorY,
                 PURPLE);
        EndMode2D();

        DrawText(renderables.leapDebugString.c_str(), 10, 10, 10, BLACK);
        EndDrawing();
    }

    CloseWindow();
}

}  // namespace Visualization