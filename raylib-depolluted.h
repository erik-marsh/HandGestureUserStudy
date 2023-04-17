// Includes raylib and undefines constants that cause compiler issues.

#include "raylib/src/raylib.h"

// Undefine numerical constants with similar names to constants in the LeapSDK.
// PI in particular has a breaking name collision with Leap::PI.
// (Doesn't math.h define M_PI with sufficient precision anyway?)
#undef PI  
#undef DEG2RAD
#undef RAD2DEG