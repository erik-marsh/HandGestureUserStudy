#include "Vector3Common.hpp"

namespace Helpers
{

Vector3Common::Vector3Common() : m_vec(Vector3{0.0f, 0.0f, 0.0f}) {}

Vector3Common::Vector3Common(float x, float y, float z) : m_vec(Vector3{x, y, z}) {}

Vector3Common::Vector3Common(Vector3 vec) : m_vec(vec) {}

Vector3Common::Vector3Common(LEAP_VECTOR vec) : m_vec(Vector3{vec.x, vec.y, vec.z}) {}

Vector3 Vector3Common::AsRaylib() const { return m_vec; }

LEAP_VECTOR Vector3Common::AsLeap() const { return LEAP_VECTOR{m_vec.x, m_vec.y, m_vec.z}; }

float Vector3Common::X() const { return m_vec.x; }
float Vector3Common::Y() const { return m_vec.y; }
float Vector3Common::Z() const { return m_vec.z; }

float Vector3Common::Magnitude() const { return Vector3Length(m_vec); }

float Vector3Common::MagnitudeSquared() const { return Vector3LengthSqr(m_vec); }

Vector3Common Vector3Common::Add(Vector3Common a, Vector3Common b)
{
    Vector3 rayA = a.AsRaylib();
    Vector3 rayB = b.AsRaylib();
    Vector3 res = Vector3Add(rayA, rayB);
    return Vector3Common(res);
}

Vector3Common Vector3Common::Subtract(Vector3Common a, Vector3Common b)
{
    Vector3 rayA = a.AsRaylib();
    Vector3 rayB = b.AsRaylib();
    Vector3 res = Vector3Subtract(rayA, rayB);
    return Vector3Common(res);
}

Vector3Common Vector3Common::ScalarMultiply(Vector3Common a, float scalar)
{
    Vector3 rayA = a.AsRaylib();
    Vector3 res = Vector3Scale(rayA, scalar);
    return Vector3Common(res);
}

float Vector3Common::DotProduct(Vector3Common a, Vector3Common b)
{
    Vector3 rayA = a.AsRaylib();
    Vector3 rayB = b.AsRaylib();
    return Vector3DotProduct(rayA, rayB);
}

Vector3Common Vector3Common::CrossProduct(Vector3Common a, Vector3Common b)
{
    Vector3 rayA = a.AsRaylib();
    Vector3 rayB = b.AsRaylib();
    Vector3 res = Vector3CrossProduct(rayA, rayB);
    return Vector3Common(res);
}

float Vector3Common::Angle(Vector3Common from, Vector3Common to)
{
    Vector3 rayFrom = from.AsRaylib();
    Vector3 rayTo = from.AsRaylib();
    return Vector3Angle(rayFrom, rayTo);
}

Vector3Common Vector3Common::ProjectOntoPlane(Vector3Common vec, Vector3Common planeNormal)
{
    Vector3 rayVec = vec.AsRaylib();
    Vector3 rayPlaneNormal = planeNormal.AsRaylib();

    float scaleFactor = Vector3DotProduct(rayVec, rayPlaneNormal);
    scaleFactor /= Vector3LengthSqr(rayPlaneNormal);
    Vector3 scaled = Vector3Scale(rayPlaneNormal, scaleFactor);
    Vector3 res = Vector3Subtract(rayVec, scaled);
    return Vector3Common(res);
}

Vector3Common Vector3Common::ProjectOntoXYPlane(Vector3Common vec)
{
    return Vector3Common(vec.m_vec.x, vec.m_vec.y, 0.0f);
}

Vector3Common Vector3Common::ProjectOntoXZPlane(Vector3Common vec)
{
    return Vector3Common(vec.m_vec.x, 0.0f, vec.m_vec.z);
}

Vector3Common Vector3Common::ProjectOntoYZPlane(Vector3Common vec)
{
    return Vector3Common(0.0f, vec.m_vec.y, vec.m_vec.z);
}


}  // namespace Helpers