#include "LeapMotionGestureProvider.hpp"

#include "../Helpers.hpp"

namespace Input
{

bool IsVectorInCone(Leap::Vector coneAxis, float coneAngle, Leap::Vector vector)
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
    float targetDist = vector.dot(coneAxis);

    // Find the radius of the cone at the target distance
    float targetConeRadius = (coneBaseRadius * targetDist) / coneHeight;
    float targetConeRadiusSquared = targetConeRadius * targetConeRadius;

    // Find the orthogonal component of the input vector to the targetDist vector
    Leap::Vector orthVector = vector - (targetDist * coneAxis);
    float orthMagnitudeSquared = orthVector.magnitudeSquared();  // squared for efficiency

    // Check if the orthoganal component falls within the target radius
    return orthMagnitudeSquared < targetConeRadiusSquared;
}

ProcessedHandState ProcessHandState(UnprocessedHandState& inState)
{
    ProcessedHandState outState{};

    // check if the hand is in the click pose
    Leap::Vector palmNormal = inState.palmNormal;

    outState.isInClickPose =
        IsVectorInCone(Leap::Vector{0.0f, -1.0f, 0.0f}, TOLERANCE_CONE_ANGLE_RADIANS, palmNormal);

    // if true return
    // click pose and mouse pose are mutually exclusive
    if (outState.isInClickPose) return outState;

    // else check if the hand is in the cursor movement pose
    Leap::Vector referenceVec =
        inState.isLeft ? Leap::Vector{1.0f, 0.0f, 0.0f} : Leap::Vector{-1.0f, 0.0f, 0.0f};

    if (!IsVectorInCone(referenceVec, TOLERANCE_CONE_ANGLE_RADIANS, palmNormal))
        return outState;

    // calculate finger angles
    Leap::Vector fingerBendPlaneNormal = inState.handDirection.cross(inState.palmNormal);

    float averageAngle = 0.0f;
    for (auto dir : inState.fingerDirections)
    {
        Leap::Vector projectedDir = Helpers::ProjectOntoPlane(dir, fingerBendPlaneNormal);
        float angle = inState.handDirection.angleTo(projectedDir);  // ?????
        averageAngle += angle;
    }

    // calculate average finger angle
    averageAngle /= inState.fingerDirections.size();

    // calculate cursor direction
    int numCursorDirections = 8;
    float sectorArcLength = (2 * Leap::PI) / numCursorDirections;

    int sectorIndex = 0;
    // yeah i have no fucking clue what this is supposed to do but it's important
    while (averageAngle > (((2 * sectorIndex) - 1) * sectorArcLength) / 2) sectorIndex++;
    // correction
    sectorIndex--;

    outState.cursorDirectionX = std::sin(sectorIndex * sectorArcLength) * referenceVec.x;
    outState.cursorDirectionY = std::cos(sectorIndex * sectorArcLength);

    return outState;
}

}  // namespace Input