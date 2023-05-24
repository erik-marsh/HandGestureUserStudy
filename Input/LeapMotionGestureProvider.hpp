#pragma once

#include <LeapC.h>

#include <Math/MathHelpers.hpp>
#include <Math/Vector3Common.hpp>
#include <array>

namespace Input::Leap
{

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

constexpr float TOLERANCE_CONE_ANGLE_DEGREES = 25.0f;
constexpr float TOLERANCE_CONE_ANGLE_RADIANS = TOLERANCE_CONE_ANGLE_DEGREES * Math::DEG_TO_RAD;

///////////////////////////////////////////////////////////////////////////////
// Structures
///////////////////////////////////////////////////////////////////////////////

struct UnprocessedHandState
{
    /// @brief Is the hand being tracked?
    bool isTracking;

    /// @brief Is this hand the left hand?
    bool isLeft;

    /// @brief The palm normal of the hand.
    Math::Vector3Common palmNormal;

    /// @brief Direction vectors of the index, middle, ring, and pinky fingertips.
    std::array<Math::Vector3Common, 4> fingerDirections;

    /// @brief Direction vector of the hand (wrist to knuckles).
    Math::Vector3Common handDirection;
};

/// Click state and movement state are mutually exclusive.
/// If isInClickPose == true, cursorDirectionX and cursorDirectionY are invalid.
/// Otherwise, they are valid.
struct ProcessedHandState
{
    /// @brief Did we register a click?
    bool isInClickPose;

    /// @brief X component of the direction to move the cursor.
    float cursorDirectionX;

    /// @brief  Y component of the direction to move the cursor.
    float cursorDirectionY;

    /// @brief Used for debug visualizations.
    float averageFingerDirectionX;

    /// @brief Used for debug visualizations.
    float averageFingerDirectionY;
};

///////////////////////////////////////////////////////////////////////////////
// Data processing
///////////////////////////////////////////////////////////////////////////////

ProcessedHandState ProcessHandState(UnprocessedHandState& inState);

}  // namespace Input::Leap