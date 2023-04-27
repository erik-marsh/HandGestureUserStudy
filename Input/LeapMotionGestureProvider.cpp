#include "LeapMotionGestureProvider.hpp"

#include "../Helpers.hpp"

namespace Input
{

ProcessedHandState ProcessHandState(UnprocessedHandState& inState)
{
    ProcessedHandState outState{};

    // check if the hand is in the click pose
    Leap::Vector palmNormal = inState.palmNormal;
    Leap::Vector xyComp = Helpers::ProjectOntoXYPlane(palmNormal);
    Leap::Vector xzComp = Helpers::ProjectOntoXZPlane(palmNormal);
    Leap::Vector yzComp = Helpers::ProjectOntoYZPlane(palmNormal);

    float xyAngle = Leap::Vector::down().angleTo(xyComp);
    float yzAngle = Leap::Vector::down().angleTo(yzComp);

    float coneAngle = 15.0f * Leap::DEG_TO_RAD;  // TODO: should be a constant
    outState.isInClickPose = xyAngle < coneAngle && yzAngle < coneAngle;

    // if true return
    // click pose and mouse pose are mutually exclusive
    if (outState.isInClickPose) return outState;

    // else check if the hand is in the cursor movement pose
    Leap::Vector referenceVec =
        inState.isLeft ? Leap::Vector{1.0f, 0.0f, 0.0f} : Leap::Vector{-1.0f, 0.0f, 0.0f};
    xyAngle = referenceVec.angleTo(xyComp);
    float xzAngle = referenceVec.angleTo(xzComp);

    // if not, return
    if (xyAngle < coneAngle && xzAngle < coneAngle) return outState;

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