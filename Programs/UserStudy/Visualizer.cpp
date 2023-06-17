#include "Visualizer.hpp"

#include <raylib.h>
#include <rcamera.h>

#include <Visualization/RaylibVisuals.hpp>
#include <iostream>
#include <sstream>

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

    constexpr int rectX = rectCenterX - (rectWidth / 2);
    constexpr int rectY = rectCenterY - (rectHeight / 2);

    std::stringstream ss;
    ss << std::boolalpha;

    while (isRunning.load())
    {
        UpdateCamera(camera3D);

        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        BeginMode3D(camera3D);
        DrawCartesianBasis();
        DrawHand(renderables.hand);
        EndMode3D();

        BeginMode2D(camera2D);
        const int cursorDirX = static_cast<int>(renderables.cursorDirX * vectorLength);
        // multiply by -1 because Y is down in screen space
        const int cursorDirY = static_cast<int>(-1.0f * renderables.cursorDirY * vectorLength);

        const int fingerDirX = static_cast<int>(renderables.avgFingerDirX * vectorLength * 0.5f);
        const int fingerDirY =
            static_cast<int>(-1.0f * renderables.avgFingerDirY * vectorLength * 0.5f);

        DrawRectangle(rectX, rectY, rectWidth, rectHeight, DARKGRAY);

        DrawLine(rectCenterX, rectCenterY, rectCenterX + cursorDirX, rectCenterY + cursorDirY,
                 YELLOW);
        DrawLine(rectCenterX, rectCenterY, rectCenterX + fingerDirX, rectCenterY + fingerDirY,
                 PURPLE);
        EndMode2D();

        const bool isLeft = renderables.hand.type == eLeapHandType_Left;
        ss.str("");  // clear stream
        ss << (isLeft ? "Left" : "Right") << " Hand:\n"
           << "    Cursor direction: (" << renderables.cursorDirX << ", " << renderables.cursorDirY
           << "); "
           << "    Clicked?: " << renderables.didClick << "\n\n";

        DrawText(ss.str().c_str(), 10, 10, 10, BLACK);
        EndDrawing();
    }

    CloseWindow();
    std::cout << "Shutting down rendering thread..." << std::endl;
}

}  // namespace Visualization