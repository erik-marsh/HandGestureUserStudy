#pragma once

#include <LeapC.h>

#include <array>

#include "../Helpers/MathHelpers.hpp"
#include "../Helpers/Vector3Common.hpp"

namespace Input
{

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

constexpr float TOLERANCE_CONE_ANGLE_DEGREES = 25.0f;
constexpr float TOLERANCE_CONE_ANGLE_RADIANS =
    TOLERANCE_CONE_ANGLE_DEGREES * Helpers::Math::DEG_TO_RAD;

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
    Helpers::Vector3Common palmNormal;

    /// @brief Direction vectors of the index, middle, ring, and pinky fingertips.
    std::array<Helpers::Vector3Common, 4> fingerDirections;

    /// @brief Direction vector of the hand (wrist to knuckles).
    Helpers::Vector3Common handDirection;
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

/// @brief Detects if a vector is in a cone.
/// @param coneAxis The axis of the cone (base to tip).
/// @param coneAngle The angle between the cone's axis and its outer surface.
/// @param vector The vector to test.
bool IsVectorInCone(Helpers::Vector3Common coneAxis, float coneAngle,
                    Helpers::Vector3Common vector);

ProcessedHandState ProcessHandState(UnprocessedHandState& inState);

}  // namespace Input