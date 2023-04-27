#pragma once

#include <array>

#include "../LeapSDK/include/Leap.h"

namespace Input
{

// namespace
// {
struct UnprocessedHandState
{
    // is the hand currently being tracked?
    bool isTracking;
    bool isLeft;

    // used to determine which gestures we need to recognize
    Leap::Vector palmNormal;

    // used to determine the direction to move the mouse
    // (this is actually post-processing)
    // std::array<float, 4> fingerAngles;
    std::array<Leap::Vector, 4> fingerDirections;

    // other values useful for debug visualizations
    Leap::Vector palmPosition;
    Leap::Vector handDirection;
};

struct ProcessedHandState
{
    // bool isTracking;
    // bool isInCursorMovementPose;
    bool isInClickPose;
    float cursorDirectionX;
    float cursorDirectionY;
};

ProcessedHandState ProcessHandState(UnprocessedHandState& inState);

// }  // namespace

class LeapMotionGestureProvider
{
   public:
   private:
};

}  // namespace Input