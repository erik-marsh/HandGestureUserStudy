#pragma once

// Grabbing math constants like M_PI
#ifdef _MSC_VER
#include <corecrt_math_defines.h>
#else
#include <cmath>
#endif

#include <Math/Vector3Common.hpp>

namespace Math
{

constexpr float _PI = M_PI;  // the underscore avoids name conflict with raylib later on
constexpr float DEG_TO_RAD = _PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / _PI;

/// @brief Detects if a vector is in a cone.
/// @param coneAxis The axis of the cone (base to tip).
/// @param coneAngle The angle between the cone's axis and its outer surface.
/// @param vector The vector to test.
bool IsVectorInCone(Math::Vector3Common coneAxis, float coneAngle, Math::Vector3Common vector);

}  // namespace Math