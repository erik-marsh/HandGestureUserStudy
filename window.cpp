#include <array>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

#include "LeapSDK/include/Leap.h"
// #include "raylib-depolluted.h"

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

#include "Debug/RaylibDebug.hpp"
#include "Debug/LeapDebug.hpp"
#include "Helpers.hpp"
#include "Input/LeapMotionGestureProvider.hpp"
#include "Input/SimulatedMouse.hpp"

int main()
{
    Debug::EventListener listener;
    Leap::Controller controller;

    controller.addListener(listener);

    InitWindow(Debug::SCREEN_WIDTH, Debug::SCREEN_HEIGHT, "something something idk");

    Camera3D camera{};
    camera.position = (Vector3){10.0f, 10.0f, 10.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    Input::SimulatedMouse mouse;

    SetTargetFPS(60);

    int frameCounter = 0;
    std::string frameString = "";

    while (!WindowShouldClose())
    {
        Debug::UpdateCamera(camera);

        Leap::Frame leapFrame = controller.frame();
        if (frameCounter == 0) frameString = Debug::StringifyFrame(leapFrame);

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
        {
            std::stringstream ss;

            ClearBackground(LIGHTGRAY);
            BeginMode3D(camera);
            {
                // Debug::DrawCartesianBasis();

                Leap::HandList hands = leapFrame.hands();
                for (auto it = hands.begin(); it != hands.end(); it++)
                {
                    Leap::Hand hand = *it;
                    Input::UnprocessedHandState state{};

                    state.isTracking = true;
                    state.isLeft = hand.isLeft();
                    state.palmNormal = hand.palmNormal();
                    state.palmPosition = hand.palmPosition();
                    state.handDirection = hand.direction();

                    Leap::FingerList fingers = hand.fingers();
                    for (auto fit = fingers.begin(); fit != fingers.end(); fit++)
                    {
                        Leap::Finger finger = *fit;
                        // type enum starts with thumb as 0
                        // since we don't care about thumbs, we subtract 1
                        int index = finger.type() - 1;
                        if (index >= 0)
                            state.fingerDirections[finger.type() - 1] = finger.direction();
                    }

                    DrawLine3D(Debug::VECTOR_ORIGIN, Helpers::Vec3LeapToRaylib(state.palmNormal),
                               BLACK);
                    Debug::DrawVectorDecomposition(Helpers::Vec3LeapToRaylib(state.palmNormal),
                                                   true);

                    Input::ProcessedHandState processed = Input::ProcessHandState(state);

                    ss << (state.isLeft ? "Left" : "Right") << " Hand:\n"
                       << "    Cursor direction: (" << processed.cursorDirectionX << ", "
                       << processed.cursorDirectionY << "); "
                       << "    Clicked?: " << processed.isInClickPose << "\n\n";
                }
            }
            EndMode3D();

            std::string output = ss.str();
            DrawText(output.c_str(), 10, 10, 10, BLACK);
        }
        EndDrawing();

        frameCounter = (frameCounter + 1) % 20;
    }

    CloseWindow();
    controller.removeListener(listener);
    return 0;
}