#pragma once

#define Font __RAYLIB_FONT_T
#include "../raylib/src/raylib.h"
#undef Font
#include "../LeapSDK/include/LeapC.h"
#include "../raylib/src/raymath.h"
#include "../raylib/src/rcamera.h"
#include "../raylib/src/rlgl.h"

// Undefine numerical constants with similar names to constants in the LeapSDK.
// PI in particular has a breaking name collision with Leap::PI.
// (Doesn't math.h define M_PI with sufficient precision anyway?)
#undef PI
#undef DEG2RAD
#undef RAD2DEG

namespace Debug
{
constexpr int SCREEN_WIDTH = 1600;
constexpr int SCREEN_HEIGHT = 900;

constexpr float CAMERA_MOVE_SPEED = 0.09f;
constexpr float CAMERA_ROTATION_SPEED = 0.03f;

constexpr float VECTOR_BASIS_LENGTH = 10.0f;

constexpr Vector3 VECTOR_ORIGIN = {0.0f, 0.0f, 0.0f};
constexpr Vector3 VECTOR_BASIS_I = {VECTOR_BASIS_LENGTH, 0.0f, 0.0f};
constexpr Vector3 VECTOR_BASIS_J = {0.0f, VECTOR_BASIS_LENGTH, 0.0f};
constexpr Vector3 VECTOR_BASIS_K = {0.0f, 0.0f, VECTOR_BASIS_LENGTH};

constexpr Color VECTOR_BASIS_I_COLOR = RED;
constexpr Color VECTOR_BASIS_J_COLOR = GREEN;
constexpr Color VECTOR_BASIS_K_COLOR = BLUE;

void DrawCartesianBasis();
void DrawVectorDecomposition(Vector3 vec, bool shouldSumVectors);
void UpdateCamera(Camera& camera);
void DrawCone(const Vector3 coneAxis, const Vector3 circleRotationAxis, const float coneAngle,
              const float coneHeight, const Color coneColor);
void DrawHand(LEAP_HAND& hand);
// vec1 and vec2 are used to define the plane,
// and they are assumed to be orthogonal in the subspace defined by that plane.
// That is, vec1 cross vec2 = the plane normal
void DrawPlane(Vector3 centerPos, Vector3 vec1, Vector3 vec2, float size, Color color);
// 3D text drawing shamelessly copied (with some modifications) from the example at
// https://www.raylib.com/examples/text/loader.html?name=text_draw_3d
void DrawText3D(__RAYLIB_FONT_T font, const char* text, Vector3 position, float fontSize,
                float fontSpacing, float lineSpacing, bool backface, Color tint,
                float rotationAngle, Vector3 rotationAxis);

}  // namespace Debug