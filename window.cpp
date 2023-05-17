#include <array>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

#include "LeapSDK/include/LeapC.h"

#define Font __RAYLIB_FONT_T
#define Rectangle __RAYLIB_RECTANGLE_T
#include "raylib/src/raylib.h"
#undef Font
#undef Rectangle
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

    int frameCounter = 0;
    std::string frameString = "";

    while (!WindowShouldClose())
    {
        Debug::UpdateCamera(camera);

        LEAP_TRACKING_EVENT *leapFrame = connection.GetFrame();
        if (frameCounter == 0) frameString = Debug::StringifyFrame(*leapFrame);

        if (IsKeyDown(KEY_E)) Input::Mouse::MoveRelative(10, 0);
        if (IsKeyDown(KEY_Q)) Input::Mouse::MoveRelative(-10, 0);
        if (IsKeyDown(KEY_F)) Input::Mouse::LeftClick();

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
            Debug::DrawHand(hand);

            // load raw state
            Input::UnprocessedHandState state{};

            state.isTracking = true;
            state.isLeft = hand.type == eLeapHandType_Left;
            state.palmNormal = Vec3(hand.palm.normal);
            state.palmPosition = Vec3(hand.palm.position);
            state.handDirection = Vec3(hand.palm.direction);

            for (int i = 1; i < 5; i++)
            {
                LEAP_DIGIT finger = hand.digits[i];
                Vec3 distalTip(finger.distal.next_joint);
                Vec3 distalBase(finger.distal.prev_joint);

                // direction of the distal bone of the finger
                state.fingerDirections[i - 1] = Vec3::Subtract(distalTip, distalBase);
            }

            // debug text drawing for hand angles
            // TODO: a lot of this is duplicated, prefer to get this in ProcessedHandState
            // TODO: need a method that transforms Leap Motion space to a usable raylib space
            const Vec3 textOrigin{4.0f, 6.0f, 0.0f};
            float averageAngle = 0.0f;

            for (int i = 1; i < 5; i++)
            {
                LEAP_DIGIT finger = hand.digits[i];
                Vec3 distalTip(finger.distal.next_joint);
                Vec3 distalBase(finger.distal.prev_joint);
                auto dir = Vec3::Subtract(distalTip, distalBase);
                Vec3 fingerBendPlaneNormal =
                    Vec3::CrossProduct(state.handDirection, state.palmNormal);

                Vec3 projectedDir = Vec3::ProjectOntoPlane(dir, fingerBendPlaneNormal);
                float angle = Vec3::Angle(state.handDirection, projectedDir) * Input::RAD_TO_DEG;

                Vec3 textPos = {textOrigin.X(), textOrigin.Y() - ((i - 1) * 0.5f), textOrigin.Z()};

                char *opt = const_cast<char *>(TextFormat("angle=%.0f", angle));
                Debug::DrawText3D(GetFontDefault(), opt, textPos.AsRaylib(), 3.0f, 1.0f, 0.0f, false, RED,
                           90.0f, {1.0f, 0.0f, 0.0f});

                Vec3 armEnd(hand.arm.next_joint);
                Vec3 normalizedTip = Vec3::Subtract(distalTip, armEnd);
                normalizedTip = Vec3::ScalarMultiply(normalizedTip, 0.01f);
                DrawLine3D(normalizedTip.AsRaylib(), textPos.AsRaylib(), WHITE);

                averageAngle += angle;
            }
            averageAngle /= 4.0f;
            char *text = const_cast<char *>(TextFormat("     average angle=%.0f", averageAngle));
            Vec3 textPos = {textOrigin.X(), textOrigin.Y() - 2.0f, textOrigin.Z()};
            Debug::DrawText3D(GetFontDefault(), text, textPos.AsRaylib(), 3.0f, 1.0f, 0.0f, false, RED,
                       90.0f, {1.0f, 0.0f, 0.0f});

            Vec3 palmPos = Vec3(hand.palm.position).AsRaylib();
            Vec3 armEnd(hand.arm.next_joint);
            palmPos = Vec3::Subtract(palmPos, armEnd);
            palmPos = Vec3::ScalarMultiply(palmPos, 0.01f);
            Vector3 centerPos = palmPos.AsRaylib();
            DrawLine3D(Debug::VECTOR_ORIGIN, centerPos, BLACK);

            float size = 3.0f;
            float opacity = 0.35f;

            Color color = GREEN;
            color.a = static_cast<unsigned char>(opacity * 255);

            Vector3 pn = Vector3Scale(Vector3Normalize(state.palmNormal.AsRaylib()), size);
            Vector3 pd = Vector3Scale(Vector3Normalize(Vec3(hand.palm.direction).AsRaylib()), size);
            Vector3 cr = Vector3Scale(Vector3Normalize(Vector3CrossProduct(pn, pd)), size);
            Vector3 crInverse = Vector3Scale(cr, -1.0f);

            Debug::DrawPlane(centerPos, pd, cr, size, color);
            Debug::DrawPlane(centerPos, pd, crInverse, size, color);

            DrawLine3D(Debug::VECTOR_ORIGIN, state.palmNormal.AsRaylib(), BLUE);

            Input::ProcessedHandState processed = Input::ProcessHandState(state);

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

        std::string output = ss.str();
        DrawText(output.c_str(), 10, 10, 10, BLACK);
        EndDrawing();

        frameCounter = (frameCounter + 1) % 20;
    }

    CloseWindow();

    return 0;
}