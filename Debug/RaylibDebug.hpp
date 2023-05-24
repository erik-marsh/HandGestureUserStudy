#pragma once

#define __this_needs_to_be_included_first_for_some_reason
#include <raylib.h>
#undef __this_needs_to_be_included_first_for_some_reason

#include <LeapC.h>
#include <raymath.h>
#include <rcamera.h>
#include <rlgl.h>

#include "../Math/Vector3Common.hpp"

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

/// @brief Draws three vectors indicating the I, J, and K bases of Cartesian 3-space.
void DrawCartesianBasis();

/// @brief Draws each component (I, J, K) of a vector.
/// @param vec The vector to decompose.
/// @param shouldSumVectors If false, each component of vec will be drawn starting at the origin.
///                         If true, each component of vec will start where the previous component
///                         ended (in I, J, K order).
void DrawVectorDecomposition(Vector3 vec, bool shouldSumVectors);

/// @brief Custom camera update rules. See implementation for details.
/// @param camera The camera to update.
void UpdateCamera(Camera& camera);

/// @brief Draws four circles of increasing radius that represent a cone.
/// @param coneAxis
/// @param circleRotationAxis
/// @param coneAngle
/// @param coneHeight
/// @param coneColor
void DrawCone(Vector3 coneAxis, Vector3 circleRotationAxis, float coneAngle, float coneHeight,
              Color coneColor);

/// @brief Draws vectors representing the bones of the fingers of a Leap Motion hand.
/// @param hand The hand to draw.
void DrawHand(LEAP_HAND& hand);

/// @brief Draws a rectangular slice of a plane.
/// @param centerPos The center point of the slice of the plane.
/// @param vec1 A vector within the plane used to define the plane's basis.
/// @param vec2 A vector within the plane used to define the plane's basis.
/// @param color The color to draw the plane.
/// @remarks vec1 and vec2 should be orthogonal to each other in the plane
/// (vec1 cross vec2 = the plane normal).
/// The plane's size is determined by the magnitudes of vec1 and vec2.
void DrawPlane(Vector3 centerPos, Vector3 vec1, Vector3 vec2, Color color);

/// @brief Annoyingly complex way to draw text in 3-space.
/// @param font
/// @param text
/// @param position
/// @param fontSize
/// @param fontSpacing
/// @param lineSpacing
/// @param backface
/// @param tint
/// @param rotationAngle
/// @param rotationAxis
/// @remarks This routine is largely and shamelessly copied from this raylib example:
/// https://www.raylib.com/examples/text/loader.html?name=text_draw_3d
/// There are a few modifications of my own.
void DrawText3D(Font font, const char* text, Vector3 position, float fontSize, float fontSpacing,
                float lineSpacing, bool backface, Color tint, float rotationAngle,
                Vector3 rotationAxis);

/// @brief Projects a vector in the Leap Motion space to a vector that is more comfortable to view
///        in the raylib space.
/// @param vec The vector to be projected.
/// @param newOrigin A vector to be considered the new origin of the space.
/// @param scaleFactor The scale factor to scale the vector down by.
/// @return The projected vector.
Math::Vector3Common ProjectLeapIntoRaylibSpace(Math::Vector3Common vec,
                                               Math::Vector3Common newOrigin, float scaleFactor);

}  // namespace Debug