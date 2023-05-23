#include "RaylibDebug.hpp"

#include <cmath>
#include <iostream>

#include "../Helpers/Vector3Common.hpp"

using Vec3 = Helpers::Vector3Common;

namespace Debug
{

void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface,
                         Color tint, float rotationAngle, Vector3 rotationAxis);

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
        DrawLine3D(VECTOR_ORIGIN, Vector3{0.0f, vec.y, 0.0f}, VECTOR_BASIS_J_COLOR);
        DrawLine3D(VECTOR_ORIGIN, Vector3{0.0f, 0.0f, vec.z}, VECTOR_BASIS_K_COLOR);
    }
}

void UpdateCamera(Camera &camera)
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

void DrawCone(Vector3 coneAxis, Vector3 circleRotationAxis, float coneAngle, float coneHeight,
              Color coneColor)
{
    const float coneBaseRadius = coneHeight * std::tan(coneAngle);

    for (int i = 1; i <= 4; i++)
    {
        float scalar = 0.25f * i;
        Vector3 scaledAxis = Vector3Scale(coneAxis, scalar * coneHeight);
        DrawCircle3D(scaledAxis, scalar * coneBaseRadius, circleRotationAxis, 90.0f, coneColor);
    }
}

void DrawHand(LEAP_HAND &hand)
{
    // some common values used throughout the function
    Vec3 armEnd(hand.arm.next_joint);
    constexpr float scaleFactor = 0.01f;

    Vec3 palmNormal = Vec3(hand.palm.normal);
    Vec3 palmDirection = Vec3(hand.palm.direction);

    // Draw all of the bones of the index, middle, ring, and pinky fingers.
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            Vec3 boneStart(hand.digits[i].bones[j].prev_joint);
            Vec3 boneEnd(hand.digits[i].bones[j].next_joint);

            boneStart = ProjectLeapIntoRaylibSpace(boneStart, armEnd, scaleFactor);
            boneEnd = ProjectLeapIntoRaylibSpace(boneEnd, armEnd, scaleFactor);

            // Since we do calculations on the distal bone, we highlight it.
            Color color = j == 3 ? RED : BLACK;
            DrawLine3D(boneStart.AsRaylib(), boneEnd.AsRaylib(), color);
        }
    }

    // Draw planes that indicate how each individual finger bend angle is being measured
    float size = 3.0f;
    float opacity = 0.35f;

    Color color = GREEN;
    color.a = static_cast<unsigned char>(opacity * 255);

    // NOTE: re-retrieval of palm normal
    Vec3 pn = Vec3::SetMagnitude(palmNormal, size);
    Vec3 pd = Vec3::SetMagnitude(palmDirection, size);
    Vec3 cr = Vec3::CrossProduct(pn, pd);
    cr = Vec3::SetMagnitude(cr, size);
    Vec3 crInverse = Vec3::ScalarMultiply(cr, -1.0f);

    DrawLine3D(Debug::VECTOR_ORIGIN, palmNormal.AsRaylib(), BLUE);

    Vec3 palmPos = Vec3(hand.palm.position);
    palmPos = ProjectLeapIntoRaylibSpace(palmPos, armEnd, scaleFactor);

    Debug::DrawPlane(palmPos.AsRaylib(), pd.AsRaylib(), cr.AsRaylib(), color);
    Debug::DrawPlane(palmPos.AsRaylib(), pd.AsRaylib(), crInverse.AsRaylib(), color);

    // Draw text to show each individual finger angle as well as the average
    const Vec3 textOrigin{4.0f, 6.0f, 0.0f};
    Font font = GetFontDefault();
    float averageAngle = 0.0f;
    constexpr float fontSize = 3.0f;
    constexpr float fontSpacing = 1.0f;
    constexpr float lineSpacing = 0.0f;
    constexpr float textRotationAngle = 90.0f;
    const Vector3 textRotationAxis{1.0f, 0.0f, 0.0f};

    for (int i = 1; i < 5; i++)
    {
        LEAP_DIGIT finger = hand.digits[i];
        Vec3 distalTip(finger.distal.next_joint);
        Vec3 distalBase(finger.distal.prev_joint);
        auto dir = Vec3::Subtract(distalTip, distalBase);
        Vec3 fingerBendPlaneNormal = Vec3::CrossProduct(palmDirection, palmNormal);

        Vec3 projectedDir = Vec3::ProjectOntoPlane(dir, fingerBendPlaneNormal);
        float angle =
            Vec3::Angle(palmDirection, projectedDir) * 57.2957795131f;  // Input::RAD_TO_DEG;

        Vec3 textPos = {textOrigin.X(), textOrigin.Y() - ((i - 1) * 0.5f), textOrigin.Z()};

        const char *opt = TextFormat("angle=%.0f", angle);
        Debug::DrawText3D(font, opt, textPos.AsRaylib(), fontSize, fontSpacing, lineSpacing, false,
                          RED, textRotationAngle, textRotationAxis);

        Vec3 normalizedTip = ProjectLeapIntoRaylibSpace(distalTip, armEnd, scaleFactor);
        DrawLine3D(normalizedTip.AsRaylib(), textPos.AsRaylib(), WHITE);

        averageAngle += angle;
    }
    averageAngle /= 4.0f;
    const char *text = TextFormat("     average angle=%.0f", averageAngle);
    Vec3 textPos = {textOrigin.X(), textOrigin.Y() - 2.0f, textOrigin.Z()};
    Debug::DrawText3D(font, text, textPos.AsRaylib(), fontSize, fontSpacing, lineSpacing, false,
                      RED, textRotationAngle, textRotationAxis);
}

void DrawPlane(Vector3 centerPos, Vector3 vec1, Vector3 vec2, Color color)
{
    Vector3 offset{(vec1.x + vec2.x) / -2.0f, (vec1.y + vec2.y) / -2.0f, (vec1.z + vec2.z) / -2.0f};
    rlPushMatrix();
    rlTranslatef(centerPos.x + offset.x, centerPos.y + offset.y, centerPos.z + offset.z);

    rlBegin(RL_QUADS);
    rlColor4ub(color.r, color.g, color.b, color.a);
    rlVertex3f(0.0f, 0.0f, 0.0f);
    rlVertex3f(vec1.x, vec1.y, vec1.z);
    rlVertex3f((vec1.x + vec2.x), (vec1.y + vec2.y), (vec1.z + vec2.z));
    rlVertex3f(vec2.x, vec2.y, vec2.z);
    rlEnd();
    rlPopMatrix();
}

// Draw codepoint at specified position in 3D space
void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface,
                         Color tint, float rotationAngle, Vector3 rotationAxis)
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

// Draw a 2D text in 3D space
void DrawText3D(Font font, const char *text, Vector3 position, float fontSize, float fontSpacing,
                float lineSpacing, bool backface, Color tint, float rotationAngle,
                Vector3 rotationAxis)
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

Vec3 ProjectLeapIntoRaylibSpace(Vec3 vec, Vec3 newOrigin, float scaleFactor)
{
    Vec3 out = Vec3::Subtract(vec, newOrigin);
    return Vec3::ScalarMultiply(out, scaleFactor);
}

}  // namespace Debug