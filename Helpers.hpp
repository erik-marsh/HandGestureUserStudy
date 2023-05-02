#pragma once

#include "raylib-depolluted.h"
#include "LeapSDK/include/Leap.h"

namespace Helpers
{
inline Vector3 ScaleRaylibVec3(const Vector3 inVec, const float scaleFactor)
{
    return Vector3{inVec.x * scaleFactor, inVec.y * scaleFactor, inVec.z * scaleFactor};
}

inline Vector3 Vec3LeapToRaylib(const Leap::Vector& inVec)
{
    Vector3 outVec = {inVec.x, inVec.y, inVec.z};
    return outVec;
}

inline Leap::Vector Vec3RaylibToLeap(const Vector3& inVec)
{
    return Leap::Vector(inVec.x, inVec.y, inVec.z);
}

inline Leap::Vector ProjectOntoXYPlane(Leap::Vector vec)
{
    vec.z = 0.0f;
    return vec;
}

inline Leap::Vector ProjectOntoXZPlane(Leap::Vector vec)
{
    vec.y = 0.0f;
    return vec;
}

inline Leap::Vector ProjectOntoYZPlane(Leap::Vector vec)
{
    vec.x = 0.0f;
    return vec;
}

inline Leap::Vector ProjectOntoPlane(Leap::Vector vec, Leap::Vector planeNormal)
{
    float scaleFactor = vec.dot(planeNormal);
    scaleFactor /= planeNormal.magnitudeSquared();
    return vec - (scaleFactor * planeNormal);
}
}  // namespace Helpers