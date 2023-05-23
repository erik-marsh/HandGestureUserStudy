#include <array>
#include <cmath>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include "LeapSDK/include/LeapC.h"

#define please_dont_move_this_clang_format_thanks
#include "raylib/src/raylib.h"
#include "raylib/src/rcamera.h"
#undef please_dont_move_this_clang_format_thanks

#include "Debug/LeapDebug.hpp"
#include "Debug/RaylibDebug.hpp"
#include "Helpers/LeapConnection.hpp"
#include "Helpers/Vector3Common.hpp"
#include "Input/LeapMotionGestureProvider.hpp"
#include "Input/SimulatedMouse.hpp"

using Vec3 = Helpers::Vector3Common;

int main()
{
    Helpers::LeapConnection connection;
    while (!connection.IsConnected()) Sleep(100);

    InitWindow(Debug::SCREEN_WIDTH, Debug::SCREEN_HEIGHT, "something something idk");

    Camera3D camera{};
    camera.position = Vector3{10.0f, 10.0f, 10.0f};
    camera.target = Vector3{0.0f, 0.0f, 0.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    bool isClickDisengaged = true;

    while (!WindowShouldClose())
    {
        Debug::UpdateCamera(camera);

        LEAP_TRACKING_EVENT *leapFrame = connection.GetFrame();

        BeginDrawing();
        std::stringstream ss;

        ClearBackground(LIGHTGRAY);
        BeginMode3D(camera);
        Debug::DrawCartesianBasis();
        bool hadClickThisFrame = false;
        bool wasLeftFacingThisFrame = false;
        bool wasRightFacingThisFrame = false;

        std::optional<Input::ProcessedHandState> currentState = std::nullopt;

        for (int i = 0; i < leapFrame->nHands; i++)
        {
            LEAP_HAND hand = leapFrame->pHands[i];
            Debug::DrawHand(hand);

            // load raw state
            Input::UnprocessedHandState state{};

            state.isTracking = true;
            state.isLeft = hand.type == eLeapHandType_Left;
            state.palmNormal = Vec3(hand.palm.normal);
            state.handDirection = Vec3(hand.palm.direction);

            for (int i = 1; i < 5; i++)
            {
                LEAP_DIGIT finger = hand.digits[i];
                Vec3 distalTip(finger.distal.next_joint);
                Vec3 distalBase(finger.distal.prev_joint);

                // direction of the distal bone of the finger
                state.fingerDirections[i - 1] = Vec3::Subtract(distalTip, distalBase);
            }

            Input::ProcessedHandState processed = Input::ProcessHandState(state);
            currentState = processed;

            ss << (state.isLeft ? "Left" : "Right") << " Hand:\n"
               << "    Cursor direction: (" << processed.cursorDirectionX << ", "
               << processed.cursorDirectionY << "); "
               << "    Clicked?: " << processed.isInClickPose << "\n\n";

            if (processed.isInClickPose) hadClickThisFrame = true;
            if (state.palmNormal.X() > 0.0f &&
                Input::IsVectorInCone(Vec3{1.0f, 0.0f, 0.0f}, Input::TOLERANCE_CONE_ANGLE_RADIANS,
                                      state.palmNormal))
                wasRightFacingThisFrame = true;
            if (state.palmNormal.X() < 0.0f &&
                Input::IsVectorInCone(Vec3{-1.0f, 0.0f, 0.0f}, Input::TOLERANCE_CONE_ANGLE_RADIANS,
                                      state.palmNormal))
                wasLeftFacingThisFrame = true;
        }

        // draw click cone
        // Color clickColor = hadClickThisFrame ? GREEN : BLACK;
        // Debug::DrawCone(Vector3{0.0f, -1.0f, 0.0f}, Debug::VECTOR_BASIS_I,
        //                 Input::TOLERANCE_CONE_ANGLE_RADIANS, 2.0f, clickColor);

        // // draw recognition cones
        // Debug::DrawCone(Vector3{1.0f, 0.0f, 0.0f}, Debug::VECTOR_BASIS_J,
        //                 Input::TOLERANCE_CONE_ANGLE_RADIANS, 2.0f,
        //                 wasRightFacingThisFrame ? GREEN : BLACK);
        // Debug::DrawCone(Vector3{-1.0f, 0.0f, 0.0f}, Debug::VECTOR_BASIS_J,
        //                 Input::TOLERANCE_CONE_ANGLE_RADIANS, 2.0f,
        //                 wasLeftFacingThisFrame ? GREEN : BLACK);
        EndMode3D();

        Camera2D camera2d{};
        camera2d.zoom = 1.0f;

        BeginMode2D(camera2d);
        constexpr int rectCenterX = 100;
        constexpr int rectCenterY = Debug::SCREEN_HEIGHT - 100;

        constexpr int rectWidth = 200;
        constexpr int rectHeight = 200;
        constexpr int vectorLength = 100;

        DrawRectangle(rectCenterX - (rectWidth / 2), rectCenterY - (rectHeight / 2),
                      rectCenterX + (rectWidth / 2), rectCenterY + (rectHeight / 2), DARKGRAY);

        if (currentState)
        {
            const int vectorX = static_cast<int>(currentState->cursorDirectionX * vectorLength);
            const int vectorY =
                static_cast<int>(-1.0f * currentState->cursorDirectionY *
                                 vectorLength);  // -1 because Y is down in screen space

            DrawLine(rectCenterX, rectCenterY, rectCenterX + vectorX, rectCenterY + vectorY,
                     YELLOW);
        }

        EndMode2D();

        std::string output = ss.str();
        DrawText(output.c_str(), 10, 10, 10, BLACK);
        EndDrawing();

        // do the input finally
        if (currentState)
        {
            // click pose and movement pose are mutually exclusive
            // poses in neither state are encoded as a relative mouse movement of (0, 0)
            if (isClickDisengaged && currentState->isInClickPose)
            {
                std::cout << "got click" << std::endl;
                Input::Mouse::LeftClick();
                // click is engaged
                // meaning: we only want to click once, not every frame
                // non-click poses will disengage the click, letting us click again
                isClickDisengaged = false;
            }
            else if (!currentState->isInClickPose)
            {
                constexpr float speed = 2.0f;
                Input::Mouse::MoveRelative(currentState->cursorDirectionX * speed,
                                           -1.0f * currentState->cursorDirectionY * speed);
                isClickDisengaged = true;
            }
        }
    }

    CloseWindow();

    return 0;
}