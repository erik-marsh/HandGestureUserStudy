#include <array>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

#include "LeapSDK/include/LeapC.h"

#define Font __RAYLIB_FONT_T
#include "raylib/src/raylib.h"
#undef Font
#include "raylib/src/rcamera.h"

// Undefine numerical constants with similar names to constants in the LeapSDK.
// PI in particular has a breaking name collision with Leap::PI.
// (Doesn't math.h define M_PI with sufficient precision anyway?)
#undef PI
#undef DEG2RAD
#undef RAD2DEG

#include "Debug/LeapDebug.hpp"
#include "Debug/RaylibDebug.hpp"
#include "Helpers/LeapConnection.hpp"
#include "Helpers/Vector3Common.hpp"
#include "Input/LeapMotionGestureProvider.hpp"
#include "Input/SimulatedMouse.hpp"

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
    Input::SimulatedMouse mouse;

    SetTargetFPS(60);

    int frameCounter = 0;
    std::string frameString = "";

    while (!WindowShouldClose())
    {
        Debug::UpdateCamera(camera);

        LEAP_TRACKING_EVENT *leapFrame = connection.GetFrame();
        if (frameCounter == 0) frameString = Debug::StringifyFrame(*leapFrame);

        if (IsKeyDown(KEY_E)) mouse.MoveRelative(10, 0);
        if (IsKeyDown(KEY_Q)) mouse.MoveRelative(-10, 0);
        if (IsKeyDown(KEY_F))
        {
            // xdo_click_window(xdoCtx)
            //  clicks might be possible through raylib
            //  the real question is: when chrome gets added into the mix, which window is the
            //  active one?
        }

        BeginDrawing();
        std::stringstream ss;

        ClearBackground(LIGHTGRAY);
        BeginMode3D(camera);
        Debug::DrawCartesianBasis();
        bool hadClickThisFrame = false;
        bool wasLeftFacingThisFrame = false;
        bool wasRightFacingThisFrame = false;

        for (int i = 0; i < leapFrame->nHands; i++)
        {
            LEAP_HAND hand = leapFrame->pHands[i];
            Input::UnprocessedHandState state{};

            state.isTracking = true;
            state.isLeft = hand.type == eLeapHandType_Left;
            state.palmNormal = Helpers::Vector3Common(hand.palm.normal);
            state.palmPosition = Helpers::Vector3Common(hand.palm.position);
            state.handDirection = Helpers::Vector3Common(hand.palm.direction);

            for (int i = 1; i < 5; i++)
            {
                LEAP_DIGIT finger = hand.digits[i];
                Helpers::Vector3Common distalTip(finger.distal.next_joint);
                Helpers::Vector3Common distalBase(finger.distal.prev_joint);
                // direction of the distal bone of the finger
                state.fingerDirections[i - 1] =
                    Helpers::Vector3Common::Subtract(distalTip, distalBase);
            }

            DrawLine3D(Debug::VECTOR_ORIGIN, state.palmNormal.AsRaylib(), BLACK);
            // Debug::DrawVectorDecomposition(Helpers::Vec3LeapToRaylib(state.palmNormal),
            //                                true);

            // Vector3 wrist =
            // Vector3Normalize(Helpers::Vec3LeapToRaylib(hand.wristPosition()));
            // DrawLine3D(Debug::VECTOR_ORIGIN, wrist, BLACK);

            // the combination of the palm normal and the vector bases
            // yields four orthogonal vectors
            // the y basis is always the negative of the palm normal
            // DrawLine3D(Debug::VECTOR_ORIGIN, (Helpers::Vec3LeapToRaylib(hand.basis().xBasis)),
            //            Debug::VECTOR_BASIS_I_COLOR);
            // DrawLine3D(Debug::VECTOR_ORIGIN, (Helpers::Vec3LeapToRaylib(hand.basis().yBasis)),
            //            Debug::VECTOR_BASIS_J_COLOR);
            // DrawLine3D(Debug::VECTOR_ORIGIN, (Helpers::Vec3LeapToRaylib(hand.basis().zBasis)),
            //            Debug::VECTOR_BASIS_K_COLOR);

            Input::ProcessedHandState processed = Input::ProcessHandState(state);

            ss << (state.isLeft ? "Left" : "Right") << " Hand:\n"
               << "    Cursor direction: (" << processed.cursorDirectionX << ", "
               << processed.cursorDirectionY << "); "
               << "    Clicked?: " << processed.isInClickPose << "\n\n";

            if (processed.isInClickPose) hadClickThisFrame = true;
            if (state.palmNormal.X() > 0.0f &&
                Input::IsVectorInCone(Helpers::Vector3Common{1.0f, 0.0f, 0.0f},
                                      Input::TOLERANCE_CONE_ANGLE_RADIANS, state.palmNormal))
                wasRightFacingThisFrame = true;
            if (state.palmNormal.X() < 0.0f &&
                Input::IsVectorInCone(Helpers::Vector3Common{-1.0f, 0.0f, 0.0f},
                                      Input::TOLERANCE_CONE_ANGLE_RADIANS, state.palmNormal))
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

        std::string output = ss.str();
        DrawText(output.c_str(), 10, 10, 10, BLACK);
        EndDrawing();

        frameCounter = (frameCounter + 1) % 20;
    }

    CloseWindow();

    return 0;
}