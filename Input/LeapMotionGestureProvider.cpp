#include "LeapMotionGestureProvider.hpp"

#include <cmath>
#include <iostream>

using Vec3 = Math::Vector3Common;

namespace Input
{

ProcessedHandState ProcessHandState(UnprocessedHandState& inState)
{
    ProcessedHandState outState{};

    // check if the hand is in the click pose
    Vec3 palmNormal = inState.palmNormal;

    outState.isInClickPose =
        Math::IsVectorInCone(Vec3{0.0f, -1.0f, 0.0f}, TOLERANCE_CONE_ANGLE_RADIANS, palmNormal);

    // if true return
    // click pose and mouse pose are mutually exclusive
    if (outState.isInClickPose) return outState;

    // else check if the hand is in the cursor movement pose
    Vec3 referenceVec = inState.isLeft ? Vec3{1.0f, 0.0f, 0.0f} : Vec3{-1.0f, 0.0f, 0.0f};

    if (!Math::IsVectorInCone(referenceVec, TOLERANCE_CONE_ANGLE_RADIANS, palmNormal))
        return outState;

    // calculate finger angles
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
        if (signingAngle > Math::_PI / 2.0f) angle *= -1.0f;

        if (angle < 0.0f)
        {
            // clamp to 0 or 180, whichever is closer
            if (angle > (Math::_PI / 2.0f) * -1.0f)
                angle = 0.0f;
            else
                angle = Math::_PI;
        }

        averageAngle += angle;
    }

    // calculate average finger angle
    averageAngle /= inState.fingerDirections.size();

    // calculate cursor direction
    constexpr int numCursorDirections = 8;                              // a.k.a. N
    constexpr int numSectors = 2 * numCursorDirections;                 // a.k.a. 2N
    constexpr float sectorArcLength = Math::_PI / numCursorDirections;  // a.k.a. phi = 2pi / 2N

    // find the index of the sector that averageAngle is currently in,
    // which is the integer part of averageAngle / sectorArcLength
    int sectorIndex = static_cast<int>(averageAngle / sectorArcLength);

    // {-1, 0} => 0phi
    // {1, 2} => 2phi
    // {3, 4} => 4phi
    // etc...
    // integer division means these factors do NOT cancel out
    int scaleFactor = ((sectorIndex + 1) / 2) * 2;

    outState.cursorDirectionX = std::sin(scaleFactor * sectorArcLength) * referenceVec.X();
    outState.cursorDirectionY = std::cos(scaleFactor * sectorArcLength);

    outState.averageFingerDirectionX = std::sin(averageAngle) * referenceVec.X();
    outState.averageFingerDirectionY = std::cos(averageAngle);

    return outState;
}

}  // namespace Input