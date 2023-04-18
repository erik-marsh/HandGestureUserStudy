#include "raylib-depolluted.h"
#include "LeapSDK/include/Leap.h"

namespace Helpers
{
    static Vector3 Vec3LeapToRaylib(const Leap::Vector& inVec)
    {
        Vector3 outVec = {inVec.x, inVec.y, inVec.z};
        return outVec;
    }

    static Leap::Vector Vec3RaylibToLeap(const Vector3& inVec)
    {
        return Leap::Vector(inVec.x, inVec.y, inVec.z);
    }
}