#include <array>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

#include "LeapSDK/include/LeapC.h"

#define Font __RAYLIB_FONT_T
#define Rectangle __RAYLIB_RECTANGLE_T
#include "raylib/src/raylib.h"
#undef Font
#undef Rectangle
#include "raylib/src/rcamera.h"
#include "raylib/src/rlgl.h"

// Undefine numerical constants with similar names to constants in the LeapSDK.
// PI in particular has a breaking name collision with Leap::PI.
// (Doesn't math.h define M_PI with sufficient precision anyway?)
#undef PI
#undef DEG2RAD
#undef RAD2DEG

#include "Debug/LeapDebug.hpp"
#include "Debug/RaylibDebug.hpp"
#include "Helpers/LeapConnection.hpp"
#include "Helpers/Vector3Common.hpp"
#include "Input/LeapMotionGestureProvider.hpp"
#include "Input/SimulatedMouse.hpp"

using Vec3 = Helpers::Vector3Common;

// 3D text drawing shamelessly copied from the example at
// https://www.raylib.com/examples/text/loader.html?name=text_draw_3d

// Draw codepoint at specified position in 3D space
static void DrawTextCodepoint3D(__RAYLIB_FONT_T font, int codepoint, Vector3 position,
                                float fontSize, bool backface, Color tint, float rotationAngle,
                                Vector3 rotationAxis)
{
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scale = fontSize / (float)font.baseSize;

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    position.x +=
        (float)(font.glyphs[index].offsetX - font.glyphPadding) / (float)font.baseSize * scale;
    position.z +=
        (float)(font.glyphs[index].offsetY - font.glyphPadding) / (float)font.baseSize * scale;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader
    // effects
    __RAYLIB_RECTANGLE_T srcRec = {font.recs[index].x - (float)font.glyphPadding,
                                   font.recs[index].y - (float)font.glyphPadding,
                                   font.recs[index].width + 2.0f * font.glyphPadding,
                                   font.recs[index].height + 2.0f * font.glyphPadding};

    float width =
        (float)(font.recs[index].width + 2.0f * font.glyphPadding) / (float)font.baseSize * scale;
    float height =
        (float)(font.recs[index].height + 2.0f * font.glyphPadding) / (float)font.baseSize * scale;

    if (font.texture.id > 0)
    {
        const float x = 0.0f;
        const float y = 0.0f;
        const float z = 0.0f;

        // normalized texture coordinates of the glyph inside the font texture (0.0f -> 1.0f)
        const float tx = srcRec.x / font.texture.width;
        const float ty = srcRec.y / font.texture.height;
        const float tw = (srcRec.x + srcRec.width) / font.texture.width;
        const float th = (srcRec.y + srcRec.height) / font.texture.height;

        // if (SHOW_LETTER_BOUNDRY) DrawCubeWiresV((Vector3){ position.x + width/2, position.y,
        // position.z + height/2}, (Vector3){ width, LETTER_BOUNDRY_SIZE, height },
        // LETTER_BOUNDRY_COLOR);

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
void DrawText3D(__RAYLIB_FONT_T font, const char *text, Vector3 position, float fontSize,
                float fontSpacing, float lineSpacing, bool backface, Color tint,
                float rotationAngle, Vector3 rotationAxis)
{
    int length =
        TextLength(text);      // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0.0f;  // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;  // Offset X to next character to draw

    float scale = fontSize / (float)font.baseSize;

    for (int i = 0; i < length;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return
        // 0x3f) but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (codepoint == '\n')
        {
            // NOTE: Fixed line spacing of 1.5 line-height
            // TODO: Support custom line spacing defined by user
            textOffsetY += scale + lineSpacing / (float)font.baseSize * scale;
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
                textOffsetX +=
                    (float)(font.recs[index].width + fontSpacing) / (float)font.baseSize * scale;
            else
                textOffsetX += (float)(font.glyphs[index].advanceX + fontSpacing) /
                               (float)font.baseSize * scale;
        }

        i += codepointByteCount;  // Move text bytes counter to next codepoint
    }
}

void DrawScuffedPlane(Vector3 centerPos, Vector3 pd, Vector3 cr, float size, Color color)
{
    using namespace Helpers;  // TODO: unscuff this
    std::cout << "centerPos: " << centerPos << ", magnitude=" << Vector3Length(centerPos)
              << std::endl;
    std::cout << "pd: " << pd << ", magnitude=" << Vector3Length(pd) << std::endl;
    std::cout << "cr: " << cr << ", magnitude=" << Vector3Length(cr) << std::endl;

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

int main()
{
    Helpers::LeapConnection connection;
    while (!connection.IsConnected()) Sleep(100);

    InitWindow(Debug::SCREEN_WIDTH, Debug::SCREEN_HEIGHT, "something something idk");

    Camera3D camera{};
    camera.position = Vector3{10.0f, 10.0f, 10.0f};
    camera.target = Vector3{0.0f, 0.0f, 0.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    int frameCounter = 0;
    std::string frameString = "";

    while (!WindowShouldClose())
    {
        Debug::UpdateCamera(camera);

        LEAP_TRACKING_EVENT *leapFrame = connection.GetFrame();
        if (frameCounter == 0) frameString = Debug::StringifyFrame(*leapFrame);

        if (IsKeyDown(KEY_E)) Input::Mouse::MoveRelative(10, 0);
        if (IsKeyDown(KEY_Q)) Input::Mouse::MoveRelative(-10, 0);
        if (IsKeyDown(KEY_F)) Input::Mouse::LeftClick();

        BeginDrawing();
        std::stringstream ss;

        ClearBackground(LIGHTGRAY);
        BeginMode3D(camera);
        Debug::DrawCartesianBasis();
        bool hadClickThisFrame = false;
        bool wasLeftFacingThisFrame = false;
        bool wasRightFacingThisFrame = false;

        for (int i = 0; i < leapFrame->nHands; i++)
        {
            LEAP_HAND hand = leapFrame->pHands[i];
            Debug::DrawHand(hand);
            Input::UnprocessedHandState state{};

            state.isTracking = true;
            state.isLeft = hand.type == eLeapHandType_Left;
            state.palmNormal = Vec3(hand.palm.normal);
            state.palmPosition = Vec3(hand.palm.position);
            state.handDirection = Vec3(hand.palm.direction);

            for (int i = 1; i < 5; i++)
            {
                LEAP_DIGIT finger = hand.digits[i];
                Vec3 distalTip(finger.distal.next_joint);
                Vec3 distalBase(finger.distal.prev_joint);

                // direction of the distal bone of the finger
                state.fingerDirections[i - 1] = Vec3::Subtract(distalTip, distalBase);
            }

            // this will be in a rotated space
            // i forgot just enough about graphics to not know how to fix this
            const Vec3 textOrigin{4.0f, 6.0f, 0.0f};
            float averageAngle = 0.0f;

            for (int i = 1; i < 5; i++)
            {
                LEAP_DIGIT finger = hand.digits[i];
                Vec3 distalTip(finger.distal.next_joint);
                Vec3 distalBase(finger.distal.prev_joint);
                auto dir = Vec3::Subtract(distalTip, distalBase);
                Vec3 fingerBendPlaneNormal =
                    Vec3::CrossProduct(state.handDirection, state.palmNormal);

                Vec3 projectedDir = Vec3::ProjectOntoPlane(dir, fingerBendPlaneNormal);
                float angle = Vec3::Angle(state.handDirection, projectedDir) * Input::RAD_TO_DEG;

                Vec3 textPos = {textOrigin.X(), textOrigin.Y() - ((i - 1) * 0.5f), textOrigin.Z()};

                char *opt = const_cast<char *>(TextFormat("angle=%.0f", angle));
                DrawText3D(GetFontDefault(), opt, textPos.AsRaylib(), 3.0f, 1.0f, 0.0f, false, RED,
                           90.0f, {1.0f, 0.0f, 0.0f});

                Vec3 armEnd(hand.arm.next_joint);
                Vec3 normalizedTip = Vec3::Subtract(distalTip, armEnd);
                normalizedTip = Vec3::ScalarMultiply(normalizedTip, 0.01f);
                DrawLine3D(normalizedTip.AsRaylib(), textPos.AsRaylib(), WHITE);

                averageAngle += angle;
            }
            averageAngle /= 4.0f;
            char *text = const_cast<char *>(TextFormat("     average angle=%.0f", averageAngle));
            Vec3 textPos = {textOrigin.X(), textOrigin.Y() - 2.0f, textOrigin.Z()};
            DrawText3D(GetFontDefault(), text, textPos.AsRaylib(), 3.0f, 1.0f, 0.0f, false, RED,
                       90.0f, {1.0f, 0.0f, 0.0f});

            Vec3 palmPos = Vec3(hand.palm.position).AsRaylib();
            Vec3 armEnd(hand.arm.next_joint);
            palmPos = Vec3::Subtract(palmPos, armEnd);
            palmPos = Vec3::ScalarMultiply(palmPos, 0.01f);
            Vector3 centerPos = palmPos.AsRaylib();
            DrawLine3D(Debug::VECTOR_ORIGIN, centerPos, BLACK);

            float size = 3.0f;
            float opacity = 0.35f;

            Color color = GREEN;
            color.a = static_cast<unsigned char>(opacity * 255);

            Vector3 pn = Vector3Scale(Vector3Normalize(state.palmNormal.AsRaylib()), size);
            Vector3 pd = Vector3Scale(Vector3Normalize(Vec3(hand.palm.direction).AsRaylib()), size);
            Vector3 cr = Vector3Scale(Vector3Normalize(Vector3CrossProduct(pn, pd)), size);
            Vector3 crInverse = Vector3Scale(cr, -1.0f);

            DrawScuffedPlane(centerPos, pd, cr, size, color);
            DrawScuffedPlane(centerPos, pd, crInverse, size, color);

            DrawLine3D(Debug::VECTOR_ORIGIN, state.palmNormal.AsRaylib(), BLUE);

            // the combination of the palm normal and the vector bases
            // yields four orthogonal vectors
            // the y basis is always the negative of the palm normal
            // DrawLine3D(Debug::VECTOR_ORIGIN, (Helpers::Vec3LeapToRaylib(hand.basis().xBasis)),
            //            Debug::VECTOR_BASIS_I_COLOR);
            // DrawLine3D(Debug::VECTOR_ORIGIN, (Helpers::Vec3LeapToRaylib(hand.basis().yBasis)),
            //            Debug::VECTOR_BASIS_J_COLOR);
            // DrawLine3D(Debug::VECTOR_ORIGIN, (Helpers::Vec3LeapToRaylib(hand.basis().zBasis)),
            //            Debug::VECTOR_BASIS_K_COLOR);

            Input::ProcessedHandState processed = Input::ProcessHandState(state);

            ss << (state.isLeft ? "Left" : "Right") << " Hand:\n"
               << "    Cursor direction: (" << processed.cursorDirectionX << ", "
               << processed.cursorDirectionY << "); "
               << "    Clicked?: " << processed.isInClickPose << "\n\n";

            if (processed.isInClickPose) hadClickThisFrame = true;
            if (state.palmNormal.X() > 0.0f &&
                Input::IsVectorInCone(Vec3{1.0f, 0.0f, 0.0f}, Input::TOLERANCE_CONE_ANGLE_RADIANS,
                                      state.palmNormal))
                wasRightFacingThisFrame = true;
            if (state.palmNormal.X() < 0.0f &&
                Input::IsVectorInCone(Vec3{-1.0f, 0.0f, 0.0f}, Input::TOLERANCE_CONE_ANGLE_RADIANS,
                                      state.palmNormal))
                wasLeftFacingThisFrame = true;
        }

        // draw click cone
        // Color clickColor = hadClickThisFrame ? GREEN : BLACK;
        // Debug::DrawCone(Vector3{0.0f, -1.0f, 0.0f}, Debug::VECTOR_BASIS_I,
        //                 Input::TOLERANCE_CONE_ANGLE_RADIANS, 2.0f, clickColor);

        // // draw recognition cones
        // Debug::DrawCone(Vector3{1.0f, 0.0f, 0.0f}, Debug::VECTOR_BASIS_J,
        //                 Input::TOLERANCE_CONE_ANGLE_RADIANS, 2.0f,
        //                 wasRightFacingThisFrame ? GREEN : BLACK);
        // Debug::DrawCone(Vector3{-1.0f, 0.0f, 0.0f}, Debug::VECTOR_BASIS_J,
        //                 Input::TOLERANCE_CONE_ANGLE_RADIANS, 2.0f,
        //                 wasLeftFacingThisFrame ? GREEN : BLACK);
        EndMode3D();

        std::string output = ss.str();
        DrawText(output.c_str(), 10, 10, 10, BLACK);
        EndDrawing();

        frameCounter = (frameCounter + 1) % 20;
    }

    CloseWindow();

    return 0;
}