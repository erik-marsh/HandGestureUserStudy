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
                std::atomic<bool>& isRunning, std::atomic<bool>& isLeapDriverActive,
                std::mutex& renderableCopyMutex)
{
    std::cout << "Starting Leap Motion driver thread..." << std::endl;

    using Clock = std::chrono::high_resolution_clock;
    using Time = std::chrono::high_resolution_clock::time_point;
    using Nanos = std::chrono::nanoseconds;

    constexpr Nanos frameTime = Nanos(1'000'000);  // 1ms

    bool isClickDisengaged = true;
    float dxAccumulator = 0.0f;
    float dyAccumulator = 0.0f;

    while (isRunning.load())
    {
        // TODO: do something else that isn't a spinlock
        while (!isLeapDriverActive.load() && isRunning.load())
            ;

        Time frameStart = Clock::now();

        LEAP_TRACKING_EVENT* leapFrame = connection.GetFrame();
        if (!leapFrame)
            continue;

        if (leapFrame->nHands == 0)
        {
            std::lock_guard<std::mutex> lock(renderableCopyMutex);
            renderables = Visualization::Renderables{};
        }
        else
        {
            // yes, we only want the most recent hand
            LEAP_HAND hand = leapFrame->pHands[leapFrame->nHands - 1];

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
            {
                std::lock_guard<std::mutex> lock(renderableCopyMutex);
                renderables.hasHand = true;
                renderables.hand = hand;
                renderables.didClick = outState.isInClickPose;
                renderables.cursorDirX = outState.cursorDirectionX;
                renderables.cursorDirY = outState.cursorDirectionY;
                renderables.avgFingerDirX = outState.averageFingerDirectionX;
                renderables.avgFingerDirY = outState.averageFingerDirectionY;
            }

            // do the input finally
            // click pose and movement pose are mutually exclusive
            // poses in neither state are encoded as a relative mouse movement of (0, 0)
            if (isClickDisengaged && outState.isInClickPose)
            {
                Input::Mouse::LeftClick();
                // click is engaged
                // meaning: we only want to click once, not every frame
                // non-click poses will disengage the click, letting us click again
                isClickDisengaged = false;
            }
            else if (!outState.isInClickPose)
            {
                // originally this speed was 2px per every 16.67ms (60Hz)
                constexpr float speed = 0.4f;
                dxAccumulator += outState.cursorDirectionX * speed;
                dyAccumulator += outState.cursorDirectionY * speed * -1.0f;

                // The Win32 mouse movement only accepts an integer number of pixels to move.
                // We harvest the integer part of the accumulator here,
                int dx = static_cast<int>(dxAccumulator);
                int dy = static_cast<int>(dyAccumulator);

                // then subtract it from the accumulator (consume the integer part),
                dxAccumulator -= static_cast<float>(dx);
                dyAccumulator -= static_cast<float>(dy);

                // and perform the movement.
                Input::Mouse::MoveRelative(dx, dy);

                // click pose and movement pose are mutually exclusive
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