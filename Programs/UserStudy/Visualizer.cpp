#include "Visualizer.hpp"

#include <raylib.h>
#include <rcamera.h>

#include <Input/LeapMotionGestureProvider.hpp>
#include <Math/MathHelpers.hpp>
#include <Visualization/RaylibVisuals.hpp>
#include <iostream>
#include <sstream>

namespace Visualization
{

constinit const char* const blueMsg = "The blue cylinder indicates the direction of your palm.";
constinit const char* const redMsg =
    "The red cylinders indicate the directions of your fingertips.";
constinit const char* const coneMsg1 = "The cones correspond to the gesture recognition poses.";
constinit const char* const coneMsg2 =
    "When your hand pose is being recognized (the tip of the blue cylinder";
constinit const char* const coneMsg3 = "is in the cone), the corresponding cone will turn green.";
constinit const char* const blackMsg = "The gray box indicates what will happen to the cursor.";
constinit const char* const purpleMsg1 =
    "The purple line shows the average direction of your fingertips,";
constinit const char* const purpleMsg2 = "mapped to the screen.";
constinit const char* const yellowMsg =
    "The yellow line shows the direction the cursor will move in.";

void MakeDirectionalTriangle(int x, int y, int rectCenterX, int rectCenterY, int delta, Color color)
{
    constexpr float pi_2 = Math::_PI / 2.0f;

    // i'm not proving this in text yet lol
    // lim_{x -> 0}(atanf(y/x)) = pi/2
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

void RenderLoop(SyncState& syncState)
{
    std::cout << "Starting rendering thread..." << std::endl;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Gesture Driver Debug Visualizer");
    SetTargetFPS(60);

    // initialize camera settings
    Camera3D camera3D{};
    camera3D.position = Vector3{0.0f, 5.0f, 5.0f};
    camera3D.target = Vector3{0.0f, 0.0f, 0.0f};
    camera3D.up = Vector3{0.0f, 1.0f, 0.0f};
    camera3D.fovy = 45.0f;
    camera3D.projection = CAMERA_PERSPECTIVE;

    Camera2D camera2D{};
    camera2D.zoom = 1.0f;

    // initialize 2D rendering settings
    constexpr int rectCenterX = 100;
    constexpr int rectCenterY = SCREEN_HEIGHT - 100;
    constexpr int rectWidth = 200;
    constexpr int rectHeight = 200;
    constexpr int vectorLength = 100;

    constexpr int rectX = rectCenterX - (rectWidth / 2);
    constexpr int rectY = rectCenterY - (rectHeight / 2);

    // initialize text rendering settings
    constexpr int fontSize = 20;
    constexpr int lineHeight = 20;
    constexpr int lineIndent = 20;

    // initialize tolerance cone settings
    constexpr unsigned char toleranceConeOpacity = 125;
    constexpr float toleranceConeLength = 2.0f;
    const float outerRadius =
        toleranceConeLength * std::tanf(Input::Leap::TOLERANCE_CONE_ANGLE_RADIANS);
    const auto positiveX = Vector3(toleranceConeLength, 0.0f, 0.0f);
    const auto negativeX = Vector3(-toleranceConeLength, 0.0f, 0.0f);
    const auto negativeY = Vector3(0.0f, -toleranceConeLength, 0.0f);

    // initialize models
    Model leapModel = LoadModel("Models/leap.glb");
    Matrix leapScale = MatrixScale(0.15f, 0.15f, 0.15f);
    Matrix leapRotation = MatrixRotateXYZ({Math::_PI / 2.0f, 0.0f, Math::_PI / 2.0f});
    Matrix leapTranslate = MatrixTranslate(0.0f, 0.0f, 20.0f);
    leapModel.transform = MatrixMultiply(leapTranslate, MatrixMultiply(leapRotation, leapScale));

    Model desktopModel = LoadModel("Models/comp.glb");
    Matrix desktopScale = MatrixScale(0.05f, 0.05f, 0.05f);
    Matrix desktopRotation = MatrixIdentity();
    Matrix desktopTranslate = MatrixTranslate(0.0f, -50.0f, -30.0f);
    desktopModel.transform =
        MatrixMultiply(desktopTranslate, MatrixMultiply(desktopRotation, desktopScale));

    std::stringstream ss;
    Renderables localRenderables{};  // Zero-initialization works for an "invalid state"

    while (syncState.isRunning.load())
    {
        {
            std::lock_guard<std::mutex> lock(syncState.renderableCopyMutex);
            localRenderables = syncState.renderables;  // Renderables struct is POD
        }

        const bool isLeft = localRenderables.hand.type == eLeapHandType_Left;
        const bool hasMovement =
            localRenderables.cursorDirX != 0.0f || localRenderables.cursorDirY != 0.0f;

        UpdateCamera(camera3D);

        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        BeginMode3D(camera3D);
        DrawModel(leapModel, VECTOR_ORIGIN, 1.0f, WHITE);
        DrawModel(desktopModel, VECTOR_ORIGIN, 1.0f, WHITE);
        DrawHand(localRenderables.hand);

        Color toleranceColorX =
            localRenderables.hasHand ? (isLeft && hasMovement ? GREEN : BLACK) : BLACK;
        Color toleranceColorMX =
            localRenderables.hasHand ? (!isLeft && hasMovement ? GREEN : BLACK) : BLACK;
        Color toleranceColorMY = localRenderables.didClick ? GREEN : BLACK;

        toleranceColorX.a = toleranceConeOpacity;
        toleranceColorMX.a = toleranceConeOpacity;
        toleranceColorMY.a = toleranceConeOpacity;

        DrawCylinderEx(VECTOR_ORIGIN, positiveX, 0.0f, outerRadius, 10, toleranceColorX);
        DrawCylinderEx(VECTOR_ORIGIN, negativeX, 0.0f, outerRadius, 10, toleranceColorMX);
        DrawCylinderEx(VECTOR_ORIGIN, negativeY, 0.0f, outerRadius, 10, toleranceColorMY);
        EndMode3D();

        BeginMode2D(camera2D);
        const int cursorDirX = static_cast<int>(localRenderables.cursorDirX * vectorLength);
        // multiply by -1 because Y is down in screen space
        const int cursorDirY = static_cast<int>(-1.0f * localRenderables.cursorDirY * vectorLength);

        const int fingerDirX =
            static_cast<int>(localRenderables.avgFingerDirX * vectorLength * 0.5f);
        const int fingerDirY =
            static_cast<int>(localRenderables.avgFingerDirY * vectorLength * -0.5f);

        DrawRectangle(rectX, rectY - (lineHeight * 1.25f), rectWidth,
                      rectHeight + (lineHeight * 1.25f), DARKGRAY);

        MakeDirectionalTriangle(cursorDirX, cursorDirY, rectCenterX, rectCenterY, 10, YELLOW);
        MakeDirectionalTriangle(fingerDirX, fingerDirY, rectCenterX, rectCenterY, 5, PURPLE);

        if (!syncState.isLeapDriverActive)
        {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, DARKGRAY);
            constexpr int posX = (SCREEN_WIDTH / 2) - 300;
            constexpr int posY = (SCREEN_HEIGHT / 2) - fontSize;
            DrawText("Leap Motion driver is inactive.", posX, posY, 2 * fontSize, WHITE);
        }

        EndMode2D();

        if (syncState.isLeapDriverActive)
        {
            ss.str("");  // clear stream
            ss << "Active hand.....: "
               << (localRenderables.hasHand ? (isLeft ? "Left" : "Right") : "None") << "\n"
               << "Clicked?........: " << (localRenderables.didClick ? "Yes" : "No");
            DrawText(ss.str().c_str(), lineIndent + (3 * SCREEN_WIDTH / 4),
                     SCREEN_HEIGHT - (3 * lineHeight), fontSize, BLACK);

            DrawText(blueMsg, lineIndent, lineHeight, fontSize, BLUE);
            DrawText(redMsg, lineIndent, lineHeight * 3, fontSize, RED);
            DrawText(blackMsg, lineIndent, lineHeight * 6, fontSize, BLACK);
            DrawText(purpleMsg1, 2 * lineIndent, lineHeight * 8, fontSize, PURPLE);
            DrawText(purpleMsg2, 2 * lineIndent, lineHeight * 10, fontSize, PURPLE);
            DrawText(yellowMsg, 2 * lineIndent, lineHeight * 12, fontSize, YELLOW);

            DrawText(coneMsg1, (SCREEN_WIDTH / 2) + lineIndent, lineHeight, fontSize, BLACK);
            DrawText(coneMsg2, (SCREEN_WIDTH / 2) + lineIndent, lineHeight * 3, fontSize, BLACK);
            DrawText(coneMsg3, (SCREEN_WIDTH / 2) + lineIndent, lineHeight * 5, fontSize, BLACK);

            DrawText("Cursor movement:", 10, rectY - lineHeight, fontSize, WHITE);
        }
        EndDrawing();
    }

    CloseWindow();
    std::cout << "Shutting down rendering thread..." << std::endl;
}

}  // namespace Visualization