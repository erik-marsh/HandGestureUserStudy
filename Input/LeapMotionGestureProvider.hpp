#pragma once

#include <array>

#include "../Helpers/Vector3Common.hpp"
#include "../LeapSDK/include/LeapC.h"

namespace Input
{

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

constexpr float DEG_TO_RAD = 0.0174532925f;   // pi / 180
constexpr float RAD_TO_DEG = 57.2957795131f;  // 180 / pi

constexpr float TOLERANCE_CONE_ANGLE_DEGREES = 25.0f;
constexpr float TOLERANCE_CONE_ANGLE_RADIANS = TOLERANCE_CONE_ANGLE_DEGREES * DEG_TO_RAD;

///////////////////////////////////////////////////////////////////////////////
// Structures
///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////
// Data processing
///////////////////////////////////////////////////////////////////////////////

bool IsVectorInCone(Helpers::Vector3Common coneAxis, float coneAngle,
                    Helpers::Vector3Common vector);
ProcessedHandState ProcessHandState(UnprocessedHandState& inState);

}  // namespace Input