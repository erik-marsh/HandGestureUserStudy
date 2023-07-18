#include "Visualizer.hpp"

#include <raylib.h>
#include <rcamera.h>

#include <Math/MathHelpers.hpp>
#include <Visualization/RaylibVisuals.hpp>
#include <iostream>
#include <sstream>

namespace Visualization
{

void MakeDirectionalTriangle(int x, int y, int rectCenterX, int rectCenterY, int delta, Color color)
{
    constexpr float pi_2 = Math::_PI / 2.0f;

    // i'm not proving this in text yet lol
    // lim_{x -> 0} = pi/2
    const float theta = x == 0 ? pi_2 : std::atanf(y / x);
    const float phi = pi_2 - theta;

    const auto t1_offset = Vector2(static_cast<int>(delta * std::cos(phi)),
                                   static_cast<int>(-1.0f * delta * std::sin(phi)));
    const auto t2_offset = Vector2(-t1_offset.x, -t1_offset.y);
    const auto t1 = Vector2(rectCenterX + t1_offset.x, rectCenterY + t1_offset.y);
    const auto t2 = Vector2(rectCenterX + t2_offset.x, rectCenterY + t2_offset.y);
    const auto t3 = Vector2(rectCenterX + x, rectCenterY + y);

    DrawTriangle(t3, t2, t1, color);
    DrawTriangle(t3, t1, t2, color);
}

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

    constexpr int fontSize = 20;
    constexpr int lineHeight = 20;
    constexpr int lineIndent = 20;

    std::stringstream ss;

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
        const int fingerDirY = static_cast<int>(renderables.avgFingerDirY * vectorLength * -0.5f);

        DrawRectangle(rectX, rectY, rectWidth, rectHeight, DARKGRAY);
        DrawRectangle(rectX, rectY - lineHeight * 1.25f, rectWidth, lineHeight * 1.25f, DARKGRAY);

        MakeDirectionalTriangle(cursorDirX, cursorDirY, rectCenterX, rectCenterY, 10, YELLOW);
        MakeDirectionalTriangle(fingerDirX, fingerDirY, rectCenterX, rectCenterY, 5, PURPLE);

        EndMode2D();

        const bool isLeft = renderables.hand.type == eLeapHandType_Left;
        ss.str("");  // clear stream
        ss << (isLeft ? "Left" : "Right") << " Hand:\n"
           << "    Cursor direction: (" << renderables.cursorDirX << ", " << renderables.cursorDirY
           << ");\n    Clicked?: " << (renderables.didClick ? "Yes" : "No");

        DrawText(ss.str().c_str(), lineIndent + SCREEN_WIDTH / 2, lineHeight, fontSize, BLACK);

        DrawText("The blue cylinder indicates the direction of your palm.", lineIndent, lineHeight,
                 fontSize, BLUE);
        DrawText("The red cylinders indicate the directions of your fingertips.", lineIndent,
                 lineHeight * 3, fontSize, RED);
        DrawText("The gray box indicates what will happen to the cursor.", lineIndent,
                 lineHeight * 6, fontSize, BLACK);
        DrawText("The purple line shows the average direction of your fingertips,", 2 * lineIndent,
                 lineHeight * 8, fontSize, PURPLE);
        DrawText("mapped to the screen.", 2 * lineIndent, lineHeight * 10, fontSize, PURPLE);
        DrawText("The yellow line shows the direction the cursor will move in.", 2 * lineIndent,
                 lineHeight * 12, fontSize, YELLOW);

        DrawText("Cursor movement:", 10, rectY - lineHeight, fontSize, WHITE);
        EndDrawing();
    }

    CloseWindow();
    std::cout << "Shutting down rendering thread..." << std::endl;
}

}  // namespace Visualization