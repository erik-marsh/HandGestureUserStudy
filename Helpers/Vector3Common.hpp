#pragma once

#include "../LeapSDK/include/LeapC.h"
#include "../raylib/src/raymath.h"

#include <ostream>

namespace Helpers
{

class Vector3Common
{
   public:
    Vector3Common();
    Vector3Common(float x, float y, float z);
    Vector3Common(Vector3 vec);
    Vector3Common(LEAP_VECTOR vec);

    Vector3 AsRaylib() const;
    LEAP_VECTOR AsLeap() const;

    float X() const;
    float Y() const;
    float Z() const;

    float Magnitude() const;
    float MagnitudeSquared() const;

    static Vector3Common Add(Vector3Common a, Vector3Common b);
    static Vector3Common Subtract(Vector3Common a, Vector3Common b);
    static Vector3Common ScalarMultiply(Vector3Common a, float scalar);
    static float DotProduct(Vector3Common a, Vector3Common b);
    static Vector3Common CrossProduct(Vector3Common a, Vector3Common b);
    static float Angle(Vector3Common from, Vector3Common to);

    static Vector3Common ProjectOntoPlane(Vector3Common vec, Vector3Common planeNormal);
    static Vector3Common ProjectOntoXYPlane(Vector3Common vec);
    static Vector3Common ProjectOntoXZPlane(Vector3Common vec);
    static Vector3Common ProjectOntoYZPlane(Vector3Common vec);

    static Vector3Common Normalize(Vector3Common vec);
    static Vector3Common SetMagnitude(Vector3Common vec, float newMagnitude);

   private:
    // We use the raylib Vector3 type internally so it can be passed to raymath functions
    Vector3 m_vec;
};

std::ostream& operator<<(std::ostream& stream, const Vector3Common& vec);
std::ostream& operator<<(std::ostream& stream, const Vector3& vec);
std::ostream& operator<<(std::ostream& stream, const LEAP_VECTOR& vec);

}  // namespace Helpers