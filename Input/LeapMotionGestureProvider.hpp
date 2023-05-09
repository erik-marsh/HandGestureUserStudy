#pragma once

#include <array>

#include "../LeapSDK/include/LeapC.h"
#include "../Helpers/Vector3Common.hpp"

namespace Input
{

constexpr float TOLERANCE_CONE_ANGLE_DEGREES = 25.0f;
constexpr float TOLERANCE_CONE_ANGLE_RADIANS = TOLERANCE_CONE_ANGLE_DEGREES * 0.0174532925f;

struct UnprocessedHandState
{
    // is the hand currently being tracked?
    bool isTracking;
    bool isLeft;

    // used to determine which gestures we need to recognize
    Helpers::Vector3Common palmNormal;

    // used to determine the direction to move the mouse
    // (this is actually post-processing)
    // std::array<float, 4> fingerAngles;
    std::array<Helpers::Vector3Common, 4> fingerDirections;

    // other values useful for debug visualizations
    Helpers::Vector3Common palmPosition;
    Helpers::Vector3Common handDirection;
};

struct ProcessedHandState
{
    // bool isTracking;
    // bool isInCursorMovementPose;
    bool isInClickPose;
    float cursorDirectionX;
    float cursorDirectionY;
};

bool IsVectorInCone(Helpers::Vector3Common coneAxis, float coneAngle, Helpers::Vector3Common vector);
ProcessedHandState ProcessHandState(UnprocessedHandState& inState);

class LeapMotionGestureProvider
{
   public:
   private:
};

}  // namespace Input