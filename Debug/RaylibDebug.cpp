#include "RaylibDebug.hpp"

#include <cmath>
#include <iostream>

#include "../Helpers/Vector3Common.hpp"

namespace Debug
{

void DrawCartesianBasis()
{
    DrawLine3D(VECTOR_ORIGIN, VECTOR_BASIS_I, VECTOR_BASIS_I_COLOR);
    DrawLine3D(VECTOR_ORIGIN, VECTOR_BASIS_J, VECTOR_BASIS_J_COLOR);
    DrawLine3D(VECTOR_ORIGIN, VECTOR_BASIS_K, VECTOR_BASIS_K_COLOR);
}

void DrawVectorDecomposition(Vector3 vec, bool shouldSumVectors)
{
    if (shouldSumVectors)
    {
        DrawLine3D(VECTOR_ORIGIN, Vector3{vec.x, 0.0f, 0.0f}, VECTOR_BASIS_I_COLOR);
        DrawLine3D(Vector3{vec.x, 0.0f, 0.0f}, Vector3{vec.x, 0.0f, vec.z}, VECTOR_BASIS_K_COLOR);
        DrawLine3D(Vector3{vec.x, 0.0f, vec.z}, vec, VECTOR_BASIS_J_COLOR);
    }
    else
    {
        DrawLine3D(VECTOR_ORIGIN, Vector3{vec.x, 0.0f, 0.0f}, VECTOR_BASIS_I_COLOR);
        DrawLine3D(VECTOR_ORIGIN, Vector3{0.0f, 0.0f, vec.z}, VECTOR_BASIS_K_COLOR);
        DrawLine3D(VECTOR_ORIGIN, Vector3{0.0f, vec.y, 0.0f}, VECTOR_BASIS_J_COLOR);
    }
}

void UpdateCamera(Camera& camera)
{
    if (IsKeyDown(KEY_DOWN)) CameraPitch(&camera, -CAMERA_ROTATION_SPEED, true, false, false);
    if (IsKeyDown(KEY_UP)) CameraPitch(&camera, CAMERA_ROTATION_SPEED, true, false, false);
    if (IsKeyDown(KEY_RIGHT)) CameraYaw(&camera, -CAMERA_ROTATION_SPEED, false);
    if (IsKeyDown(KEY_LEFT)) CameraYaw(&camera, CAMERA_ROTATION_SPEED, false);

    if (IsKeyDown(KEY_W)) CameraMoveForward(&camera, CAMERA_MOVE_SPEED, true);
    if (IsKeyDown(KEY_A)) CameraMoveRight(&camera, -CAMERA_MOVE_SPEED, true);
    if (IsKeyDown(KEY_S)) CameraMoveForward(&camera, -CAMERA_MOVE_SPEED, true);
    if (IsKeyDown(KEY_D)) CameraMoveRight(&camera, CAMERA_MOVE_SPEED, true);
    if (IsKeyDown(KEY_SPACE)) CameraMoveUp(&camera, CAMERA_MOVE_SPEED);
    if (IsKeyDown(KEY_LEFT_SHIFT)) CameraMoveUp(&camera, -CAMERA_MOVE_SPEED);

    if (IsKeyDown(KEY_KP_1))
    {
        camera.target = Vector3{0.0f, 0.0f, 0.0f};
        camera.up = Vector3{0.0f, 1.0f, 0.0f};
        camera.position = Vector3{10.0f, 0.0f, 0.0f};
        std::cout << "Looking at YZ plane (pos=(10,0,0))" << std::endl;
    }

    if (IsKeyDown(KEY_KP_2))
    {
        camera.target = Vector3{0.0f, 0.0f, 0.0f};
        camera.up = Vector3{0.0f, 1.0f, 0.0f};
        camera.position = Vector3{0.0f, 0.0f, 10.0f};
        std::cout << "Looking at XY plane (pos=(0,0,10))" << std::endl;
    }

    if (IsKeyDown(KEY_KP_3))
    {
        camera.target = Vector3{0.0f, 0.0f, 0.0f};
        camera.up = Vector3{0.0f, 1.0f, 0.0f};
        camera.position = Vector3{1.0f, 10.0f, 1.0f};
        std::cout << "Looking at XZ plane from above (pos=(1,10,1))" << std::endl;
    }

    if (IsKeyDown(KEY_KP_4))
    {
        camera.target = Vector3{0.0f, 0.0f, 0.0f};
        camera.up = Vector3{0.0f, 1.0f, 0.0f};
        camera.position = Vector3{1.0f, -10.0f, 1.0f};
        std::cout << "Looking at XZ plane from below (pos=(1,-10,1))" << std::endl;
    }
}

void DrawCone(const Vector3 coneAxis, const Vector3 circleRotationAxis, const float coneAngle,
              const float coneHeight, const Color coneColor)
{
    const float coneBaseRadius = coneHeight * std::tan(coneAngle);

    for (int i = 1; i <= 4; i++)
    {
        float scalar = 0.25f * i;
        Vector3 scaledAxis = Vector3Scale(coneAxis, scalar * coneHeight);
        DrawCircle3D(scaledAxis, scalar * coneBaseRadius, circleRotationAxis, 90.0f, coneColor);
    }
}

void DrawHand(LEAP_HAND& hand)
{
    Helpers::Vector3Common armEnd(hand.arm.next_joint);

    // Draw all of the bones of the index, middle, ring, and pinky fingers.
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            Helpers::Vector3Common boneStart(hand.digits[i].bones[j].prev_joint);
            Helpers::Vector3Common boneEnd(hand.digits[i].bones[j].next_joint);

            boneStart = Helpers::Vector3Common::Subtract(boneStart, armEnd);
            boneEnd = Helpers::Vector3Common::Subtract(boneEnd, armEnd);

            constexpr float scaleFactor = 0.01f;
            boneStart = Helpers::Vector3Common::ScalarMultiply(boneStart, scaleFactor);
            boneEnd = Helpers::Vector3Common::ScalarMultiply(boneEnd, scaleFactor);

            // Since we do calculations on the distal bone, we highlight it.
            Color color = j == 3 ? RED : BLACK;
            DrawLine3D(boneStart.AsRaylib(), boneEnd.AsRaylib(), color);
        }
    }
}

void DrawPlane(Vector3 centerPos, Vector3 pd, Vector3 cr, float size, Color color)
{
    Vector3 offset{(pd.x + cr.x) / -2.0f, (pd.y + cr.y) / -2.0f, (pd.z + cr.z) / -2.0f};
    rlPushMatrix();
    rlTranslatef(centerPos.x + offset.x, centerPos.y + offset.y, centerPos.z + offset.z);

    rlBegin(RL_QUADS);
    rlColor4ub(color.r, color.g, color.b, color.a);
    rlVertex3f(0.0f, 0.0f, 0.0f);
    rlVertex3f(pd.x, pd.y, pd.z);
    rlVertex3f((pd.x + cr.x), (pd.y + cr.y), (pd.z + cr.z));
    rlVertex3f(cr.x, cr.y, cr.z);
    rlEnd();
    rlPopMatrix();
}

// This helper should not be accessible outside the translation unit it is defined in.
namespace
{

// Draw codepoint at specified position in 3D space
void DrawTextCodepoint3D(__RAYLIB_FONT_T font, int codepoint, Vector3 position, float fontSize,
                         bool backface, Color tint, float rotationAngle, Vector3 rotationAxis)
{
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    const int index = GetGlyphIndex(font, codepoint);
    const float scale = fontSize / static_cast<float>(font.baseSize);

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    position.x += static_cast<float>(font.glyphs[index].offsetX - font.glyphPadding) /
                  static_cast<float>(font.baseSize) * scale;
    position.z += static_cast<float>(font.glyphs[index].offsetY - font.glyphPadding) /
                  static_cast<float>(font.baseSize) * scale;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader
    // effects
    const Rectangle srcRec = {font.recs[index].x - static_cast<float>(font.glyphPadding),
                              font.recs[index].y - static_cast<float>(font.glyphPadding),
                              font.recs[index].width + 2.0f * font.glyphPadding,
                              font.recs[index].height + 2.0f * font.glyphPadding};

    const float width = static_cast<float>(font.recs[index].width + 2.0f * font.glyphPadding) /
                        static_cast<float>(font.baseSize) * scale;
    const float height = static_cast<float>(font.recs[index].height + 2.0f * font.glyphPadding) /
                         static_cast<float>(font.baseSize) * scale;

    if (font.texture.id > 0)
    {
        constexpr float x = 0.0f;
        constexpr float y = 0.0f;
        constexpr float z = 0.0f;

        // normalized texture coordinates of the glyph inside the font texture (0.0f -> 1.0f)
        const float tx = srcRec.x / font.texture.width;
        const float ty = srcRec.y / font.texture.height;
        const float tw = (srcRec.x + srcRec.width) / font.texture.width;
        const float th = (srcRec.y + srcRec.height) / font.texture.height;

        rlCheckRenderBatchLimit(4 + 4 * backface);
        rlSetTexture(font.texture.id);

        rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);
        rlRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);  // added this

        rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);

        // Front Face
        rlNormal3f(0.0f, 1.0f, 0.0f);          // Normal Pointing Up
        rlTexCoord2f(tx, ty);
        rlVertex3f(x, y, z);                   // Top Left Of The Texture and Quad
        rlTexCoord2f(tx, th);
        rlVertex3f(x, y, z + height);          // Bottom Left Of The Texture and Quad
        rlTexCoord2f(tw, th);
        rlVertex3f(x + width, y, z + height);  // Bottom Right Of The Texture and Quad
        rlTexCoord2f(tw, ty);
        rlVertex3f(x + width, y, z);           // Top Right Of The Texture and Quad

        if (backface)
        {
            // Back Face
            rlNormal3f(0.0f, -1.0f, 0.0f);         // Normal Pointing Down
            rlTexCoord2f(tx, ty);
            rlVertex3f(x, y, z);                   // Top Right Of The Texture and Quad
            rlTexCoord2f(tw, ty);
            rlVertex3f(x + width, y, z);           // Top Left Of The Texture and Quad
            rlTexCoord2f(tw, th);
            rlVertex3f(x + width, y, z + height);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(tx, th);
            rlVertex3f(x, y, z + height);          // Bottom Right Of The Texture and Quad
        }
        rlEnd();
        rlPopMatrix();

        rlSetTexture(0);
    }
}

}  // namespace

// Draw a 2D text in 3D space
void DrawText3D(__RAYLIB_FONT_T font, const char* text, Vector3 position, float fontSize,
                float fontSpacing, float lineSpacing, bool backface, Color tint,
                float rotationAngle, Vector3 rotationAxis)
{
    const int length =
        TextLength(text);      // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0.0f;  // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;  // Offset X to next character to draw

    float scale = fontSize / static_cast<float>(font.baseSize);

    for (int i = 0; i < length;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        const int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        const int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return
        // 0x3f) but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (codepoint == '\n')
        {
            // NOTE: Fixed line spacing of 1.5 line-height
            // TODO: Support custom line spacing defined by user
            textOffsetY += scale + lineSpacing / static_cast<float>(font.baseSize) * scale;
            textOffsetX = 0.0f;
        }
        else
        {
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                DrawTextCodepoint3D(
                    font, codepoint,
                    {position.x + textOffsetX, position.y, position.z + textOffsetY}, fontSize,
                    backface, tint, rotationAngle, rotationAxis);
            }

            if (font.glyphs[index].advanceX == 0)
                textOffsetX += static_cast<float>(font.recs[index].width + fontSpacing) /
                               static_cast<float>(font.baseSize) * scale;
            else
                textOffsetX += static_cast<float>(font.glyphs[index].advanceX + fontSpacing) /
                               static_cast<float>(font.baseSize) * scale;
        }

        i += codepointByteCount;  // Move text bytes counter to next codepoint
    }
}

}  // namespace Debug