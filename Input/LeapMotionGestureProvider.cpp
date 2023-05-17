#include "LeapMotionGestureProvider.hpp"

#pragma message("NOTE: Using MSVC-specific headers in this translation unit")
#include <corecrt_math_defines.h>

#include <cmath>

#include <iostream>

using Vec3 = Helpers::Vector3Common;

namespace Input
{

bool IsVectorInCone(Vec3 coneAxis, float coneAngle, Vec3 vector)
{
    // We assume all inputs are unit vectors (length 1)
    // To accomodate this, we assume that the cone has a height of 2
    // We could probably get away with a height of 1 but I'm too paranoid for that
    constexpr float coneHeight = 2;

    // This function implements an algorithm that detects if a point is inside a cone.
    // All of our vectors start at the origin, so we can assume that they are points.
    // The algorithm itself is trivial (read: I don't want to explain it right now).
    // However, we must do one step of preprocessing to find the radius of the cone's base given an
    // angle. That's what this step is:
    float coneBaseRadius = coneHeight * std::tan(coneAngle);

    // Find the projection of the input vector against the cone's axis
    float targetDist = Vec3::DotProduct(vector, coneAxis);  // vector.dot(coneAxis);

    // Find the radius of the cone at the target distance
    float targetConeRadius = (coneBaseRadius * targetDist) / coneHeight;
    float targetConeRadiusSquared = targetConeRadius * targetConeRadius;

    // Find the orthogonal component of the input vector to the targetDist vector
    // LEAP_VECTOR orthVector = vector - (targetDist * coneAxis);
    Vec3 orthVector = Vec3::Subtract(vector, Vec3::ScalarMultiply(coneAxis, targetDist));
    // float orthMagnitudeSquared = orthVector.magnitudeSquared();  // squared for efficiency
    float orthMagnitudeSquared = orthVector.MagnitudeSquared();

    // Check if the orthoganal component falls within the target radius
    return orthMagnitudeSquared < targetConeRadiusSquared;
}

ProcessedHandState ProcessHandState(UnprocessedHandState& inState)
{
    ProcessedHandState outState{};

    // check if the hand is in the click pose
    Vec3 palmNormal = inState.palmNormal;

    outState.isInClickPose =
        IsVectorInCone(Vec3{0.0f, -1.0f, 0.0f}, TOLERANCE_CONE_ANGLE_RADIANS, palmNormal);

    // if true return
    // click pose and mouse pose are mutually exclusive
    if (outState.isInClickPose) return outState;

    // else check if the hand is in the cursor movement pose
    Vec3 referenceVec = inState.isLeft ? Vec3{1.0f, 0.0f, 0.0f} : Vec3{-1.0f, 0.0f, 0.0f};

    if (!IsVectorInCone(referenceVec, TOLERANCE_CONE_ANGLE_RADIANS, palmNormal)) return outState;

    // calculate finger angles
    // inState.handDirection.cross(inState.palmNormal);
    Vec3 fingerBendPlaneNormal = Vec3::CrossProduct(inState.handDirection, inState.palmNormal);

    float averageAngle = 0.0f;
    float averageSigningAngle = 0.0f;
    for (auto dir : inState.fingerDirections)
    {
        Vec3 projectedDir = Vec3::ProjectOntoPlane(dir, fingerBendPlaneNormal);

        // this is some weird hack to make signed angles work
        // raylib does not return signed angles from its vector angle method
        // but we want signed angles so we know when to clamp to 0 and 180 degrees
        // (since one hand can only be responsible for 180 degrees of movement)
        // we can trivially figure this out, however
        // if the angle from the palm normal is > 90 degrees,
        // then we can safely invert the angle.
        // there might be a bit of imprecision, but this works pretty well.
        float angle = Vec3::Angle(inState.handDirection, projectedDir);
        float signingAngle = Vec3::Angle(inState.palmNormal, projectedDir);
        if (signingAngle > M_PI / 2.0f)
            angle *= -1.0f;

        if (angle < 0.0f)
        {
            // clamp to 0 or 180, whichever is closer
            if (angle > (M_PI / 2.0f) * -1.0f)
                angle = 0.0f;
            else
                angle = M_PI;
        }

        averageAngle += angle;
    }

    // calculate average finger angle
    averageAngle /= inState.fingerDirections.size();

    // calculate cursor direction
    int numCursorDirections = 8;
    float sectorArcLength = (2 * M_PI) / numCursorDirections;

    int sectorIndex = 0;
    // yeah i have no fucking clue what this is supposed to do but it's important
    while (averageAngle > (((2 * sectorIndex) - 1) * sectorArcLength) / 2) sectorIndex++;
    // correction
    sectorIndex--;

    outState.cursorDirectionX = std::sin(sectorIndex * sectorArcLength) * referenceVec.X();
    outState.cursorDirectionY = std::cos(sectorIndex * sectorArcLength);

    return outState;
}

}  // namespace Input