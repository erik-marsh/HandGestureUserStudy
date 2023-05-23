#pragma once

// Grabbing math constants like M_PI
#ifdef _MSC_VER
#include <corecrt_math_defines.h>
#else
#include <cmath>
#endif

namespace Helpers::Math
{

constexpr float _PI = M_PI;  // avoids name conflict with raylib later on
constexpr float DEG_TO_RAD = _PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / _PI;

}  // namespace Helpers::Math