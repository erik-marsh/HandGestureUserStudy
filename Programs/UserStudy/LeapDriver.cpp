#include "LeapDriver.hpp"

#include <Input/LeapMotionGestureProvider.hpp>
#include <Input/SimulatedMouse.hpp>
#include <Math/Vector3Common.hpp>
#include <chrono>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>

using Vec3 = Math::Vector3Common;

namespace Input
{

void DriverLoop(Leap::LeapConnection& connection, Visualization::Renderables& renderables,
                std::atomic<bool>& isRunning)
{
    std::cout << "Starting Leap Motion driver thread..." << std::endl;

    using Clock = std::chrono::high_resolution_clock;
    using Time = std::chrono::high_resolution_clock::time_point;
    using Nanos = std::chrono::nanoseconds;

    constexpr Nanos frameTime = Nanos(1'000'000);  // 1ms

    bool isClickDisengaged = true;

    while (isRunning.load())
    {
        Time frameStart = Clock::now();

        // clear out renderables
        // TODO: investigate whether or not this needs to be synchronized with a mutex
        // if unsynced i would imagine it leads to frame tearing (not a huge deal here),
        // but we may run into issues when reading strings
        renderables.leapDebugString = "";
        renderables.hands.clear();
        renderables.averageFingerDirectionX = 0.0f;
        renderables.averageFingerDirectionY = 0.0f;
        renderables.cursorDirectionX = 0.0f;
        renderables.cursorDirectionY = 0.0f;

        LEAP_TRACKING_EVENT* leapFrame = connection.GetFrame();
        if (!leapFrame)
            continue;

        std::optional<Leap::ProcessedHandState> currentState = std::nullopt;
        bool isLeft = true;

        for (int i = 0; i < leapFrame->nHands; i++)
        {
            LEAP_HAND hand = leapFrame->pHands[i];
            renderables.hands.push_back(hand);

            Leap::UnprocessedHandState inState{};
            inState.isTracking = true;
            inState.isLeft = hand.type == eLeapHandType_Left;
            inState.palmNormal = Vec3(hand.palm.normal);
            inState.handDirection = Vec3(hand.palm.direction);

            for (int i = 1; i < 5; i++)
            {
                LEAP_DIGIT finger = hand.digits[i];
                Vec3 distalTip(finger.distal.next_joint);
                Vec3 distalBase(finger.distal.prev_joint);

                inState.fingerDirections[i - 1] = Vec3::Subtract(distalTip, distalBase);
            }

            Leap::ProcessedHandState outState = Leap::ProcessHandState(inState);
            currentState = outState;
            isLeft = inState.isLeft;
        }

        if (currentState)
        {
            renderables.cursorDirectionX = currentState->cursorDirectionX;
            renderables.cursorDirectionY = currentState->cursorDirectionY;
            renderables.averageFingerDirectionX = currentState->averageFingerDirectionX;
            renderables.averageFingerDirectionY = currentState->averageFingerDirectionY;

            std::stringstream ss;
            ss << (isLeft ? "Left" : "Right") << " Hand:\n"
               << "    Cursor direction: (" << currentState->cursorDirectionX << ", "
               << currentState->cursorDirectionY << "); "
               << "    Clicked?: " << currentState->isInClickPose << "\n\n";
            renderables.leapDebugString = ss.str();
        }

        // do the input finally
        if (currentState)
        {
            // click pose and movement pose are mutually exclusive
            // poses in neither state are encoded as a relative mouse movement of (0, 0)
            if (isClickDisengaged && currentState->isInClickPose)
            {
                Input::Mouse::LeftClick();
                // click is engaged
                // meaning: we only want to click once, not every frame
                // non-click poses will disengage the click, letting us click again
                isClickDisengaged = false;
            }
            else if (!currentState->isInClickPose)
            {
                // originally this speed was 2px per every 16.67ms (60Hz)
                constexpr float speed = 2.0f;
                Input::Mouse::MoveRelative(currentState->cursorDirectionX * speed,
                                           -1.0f * currentState->cursorDirectionY * speed);
                isClickDisengaged = true;
            }
        }

        Time frameEnd = Clock::now();
        Nanos processingTime = frameEnd - frameStart;
        if (processingTime.count() > 0)
            std::this_thread::sleep_for(frameTime - processingTime);
    }

    std::cout << "Shutting down Leap Motion driver thread..." << std::endl;
}

}  // namespace Input