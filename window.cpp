#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <array>

#include "LeapSDK/include/Leap.h"
// #include "raylib-depolluted.h"

#include "raylib/src/raylib.h"
#include "raylib/src/rcamera.h"

// Undefine numerical constants with similar names to constants in the LeapSDK.
// PI in particular has a breaking name collision with Leap::PI.
// (Doesn't math.h define M_PI with sufficient precision anyway?)
#undef PI  
#undef DEG2RAD
#undef RAD2DEG

#include "Helpers.hpp"

class SampleListener : public Leap::Listener
{
   public:
    virtual void onInit(const Leap::Controller&);
    virtual void onConnect(const Leap::Controller&);
    virtual void onDisconnect(const Leap::Controller&);
    virtual void onExit(const Leap::Controller&);
    virtual void onFrame(const Leap::Controller&);
    virtual void onFocusGained(const Leap::Controller&);
    virtual void onFocusLost(const Leap::Controller&);
    virtual void onDeviceChange(const Leap::Controller&);
    virtual void onServiceConnect(const Leap::Controller&);
    virtual void onServiceDisconnect(const Leap::Controller&);

   private:
};

const std::array<std::string, 5> fingerNames = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const std::array<std::string, 4> boneNames = {"Metacarpal", "Proximal", "Middle", "Distal"};
const std::array<std::string, 4> stateNames = {"STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END"};

void SampleListener::onInit(const Leap::Controller& controller)
{
    std::cout << "[LeapMotion Controller] Initialized" << std::endl;
}

void SampleListener::onConnect(const Leap::Controller& controller)
{
    std::cout << "[LeapMotion Controller] Connected" << std::endl;
}

void SampleListener::onDisconnect(const Leap::Controller& controller)
{
    // Note: not dispatched when running in a debugger.
    std::cout << "[LeapMotion Controller] Disconnected" << std::endl;
}

void SampleListener::onExit(const Leap::Controller& controller)
{
    std::cout << "[LeapMotion Controller] Exited" << std::endl;
}

void SampleListener::onFrame(const Leap::Controller& controller)
{
    return;
    // Get the most recent frame and report some basic information
    const Leap::Frame frame = controller.frame();
    // std::cout << "Frame id: " << frame.id() << ", timestamp: " << frame.timestamp()
    //           << ", hands: " << frame.hands().count()
    //           << ", extended fingers: " << frame.fingers().extended().count()
    //           << ", tools: " << frame.tools().count() << ", gestures: " <<
    //           frame.gestures().count()
    //           << std::endl;

    Leap::HandList hands = frame.hands();
    for (Leap::HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl)
    {
        // Get the first hand
        const Leap::Hand hand = *hl;
        std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
        std::cout << std::string(2, ' ') << handType << ", id: " << hand.id()
                  << ", palm position: " << hand.palmPosition() << std::endl;
        // Get the hand's normal vector and direction
        const Leap::Vector normal = hand.palmNormal();
        const Leap::Vector direction = hand.direction();

        // Calculate the hand's pitch, roll, and yaw angles
        std::cout << std::string(2, ' ') << "pitch: " << direction.pitch() * Leap::RAD_TO_DEG
                  << " degrees, "
                  << "roll: " << normal.roll() * Leap::RAD_TO_DEG << " degrees, "
                  << "yaw: " << direction.yaw() * Leap::RAD_TO_DEG << " degrees" << std::endl;

        // Get the Arm bone
        Leap::Arm arm = hand.arm();
        std::cout << std::string(2, ' ') << "Arm direction: " << arm.direction()
                  << " wrist position: " << arm.wristPosition()
                  << " elbow position: " << arm.elbowPosition() << std::endl;

        // Get fingers
        const Leap::FingerList fingers = hand.fingers();
        for (Leap::FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl)
        {
            const Leap::Finger finger = *fl;
            std::cout << std::string(4, ' ') << fingerNames[finger.type()]
                      << " finger, id: " << finger.id() << ", length: " << finger.length()
                      << "mm, width: " << finger.width() << std::endl;

            // Get finger bones
            for (int b = 0; b < 4; ++b)
            {
                Leap::Bone::Type boneType = static_cast<Leap::Bone::Type>(b);
                Leap::Bone bone = finger.bone(boneType);
                std::cout << std::string(6, ' ') << boneNames[boneType]
                          << " bone, start: " << bone.prevJoint() << ", end: " << bone.nextJoint()
                          << ", direction: " << bone.direction() << std::endl;
            }
        }
    }

    // // Get tools
    // const Leap::ToolList tools = frame.tools();
    // for (Leap::ToolList::const_iterator tl = tools.begin(); tl != tools.end(); ++tl)
    // {
    //     const Leap::Tool tool = *tl;
    //     std::cout << std::string(2, ' ') << "Tool, id: " << tool.id()
    //               << ", position: " << tool.tipPosition() << ", direction: " << tool.direction()
    //               << std::endl;
    // }

    // Get gestures
    // const Leap::GestureList gestures = frame.gestures();
    // for (int g = 0; g < gestures.count(); ++g)
    // {
    //     Leap::Gesture gesture = gestures[g];

    //     switch (gesture.type())
    //     {
    //         case Leap::Gesture::TYPE_CIRCLE:
    //         {
    //             Leap::CircleGesture circle = gesture;
    //             std::string clockwiseness;

    //             if (circle.pointable().direction().angleTo(circle.normal()) <= Leap::PI / 2)
    //             {
    //                 clockwiseness = "clockwise";
    //             }
    //             else
    //             {
    //                 clockwiseness = "counterclockwise";
    //             }

    //             // Calculate angle swept since last frame
    //             float sweptAngle = 0;
    //             if (circle.state() != Leap::Gesture::STATE_START)
    //             {
    //                 Leap::CircleGesture previousUpdate =
    //                     Leap::CircleGesture(controller.frame(1).gesture(circle.id()));
    //                 sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * Leap::PI;
    //             }
    //             std::cout << std::string(2, ' ') << "Circle id: " << gesture.id()
    //                       << ", state: " << stateNames[gesture.state()]
    //                       << ", progress: " << circle.progress() << ", radius: " <<
    //                       circle.radius()
    //                       << ", angle " << sweptAngle * Leap::RAD_TO_DEG << ", " << clockwiseness
    //                       << std::endl;
    //             break;
    //         }
    //         case Leap::Gesture::TYPE_SWIPE:
    //         {
    //             Leap::SwipeGesture swipe = gesture;
    //             std::cout << std::string(2, ' ') << "Swipe id: " << gesture.id()
    //                       << ", state: " << stateNames[gesture.state()]
    //                       << ", direction: " << swipe.direction() << ", speed: " << swipe.speed()
    //                       << std::endl;
    //             break;
    //         }
    //         case Leap::Gesture::TYPE_KEY_TAP:
    //         {
    //             Leap::KeyTapGesture tap = gesture;
    //             std::cout << std::string(2, ' ') << "Key Tap id: " << gesture.id()
    //                       << ", state: " << stateNames[gesture.state()]
    //                       << ", position: " << tap.position() << ", direction: " <<
    //                       tap.direction()
    //                       << std::endl;
    //             break;
    //         }
    //         case Leap::Gesture::TYPE_SCREEN_TAP:
    //         {
    //             Leap::ScreenTapGesture screentap = gesture;
    //             std::cout << std::string(2, ' ') << "Screen Tap id: " << gesture.id()
    //                       << ", state: " << stateNames[gesture.state()]
    //                       << ", position: " << screentap.position()
    //                       << ", direction: " << screentap.direction() << std::endl;
    //             break;
    //         }
    //         default:
    //             std::cout << std::string(2, ' ') << "Unknown gesture type." << std::endl;
    //             break;
    //     }
    // }

    if (!frame.hands().isEmpty())  // || !gestures.isEmpty())
    {
        std::cout << std::endl;
    }
}

void SampleListener::onFocusGained(const Leap::Controller& controller)
{
    std::cout << "[LeapMotion Controller] Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Leap::Controller& controller)
{
    std::cout << "[LeapMotion Controller] Focus Lost" << std::endl;
}

void SampleListener::onDeviceChange(const Leap::Controller& controller)
{
    std::cout << "[LeapMotion Controller] Device Changed" << std::endl;
    const Leap::DeviceList devices = controller.devices();

    for (int i = 0; i < devices.count(); ++i)
    {
        std::cout << "id: " << devices[i].toString() << std::endl;
        std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false")
                  << std::endl;
    }
}

void SampleListener::onServiceConnect(const Leap::Controller& controller)
{
    std::cout << "[LeapMotion Controller] Service Connected" << std::endl;
}

void SampleListener::onServiceDisconnect(const Leap::Controller& controller)
{
    std::cout << "[LeapMotion Controller] Service Disconnected" << std::endl;
}

std::string StringifyFrame(Leap::Frame& frame)
{
    std::stringstream ss;
    ss << "Frame id: " << frame.id() << "\nTimestamp: " << frame.timestamp()
       << "\nHands: " << frame.hands().count()
       << "\nExtended fingers: " << frame.fingers().extended().count() << "\n";

    auto hands = frame.hands();
    for (auto handIter = hands.begin(); handIter != hands.end(); handIter++)
    {
        auto hand = *handIter;

        std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
        const Leap::Vector normal = hand.palmNormal();
        const Leap::Vector direction = hand.direction();

        ss << handType << " (id: " << hand.id() << ")\n"
           << "  palm position: " << hand.palmPosition() << "\n"
           << "  palm normal: " << normal << "\n"
           << "  hand direction: " << direction << "\n";

        // Calculate the hand's pitch, roll, and yaw angles
        ss << "  pitch: " << direction.pitch() * Leap::RAD_TO_DEG << " degrees\n"
           << "  roll: " << normal.roll() * Leap::RAD_TO_DEG << " degrees\n"
           << "  yaw: " << direction.yaw() * Leap::RAD_TO_DEG << " degrees\n";

        // Get the Arm bone
        Leap::Arm arm = hand.arm();
        ss << "  Arm direction: " << arm.direction() << "\n"
           << "  wrist position: " << arm.wristPosition() << "\n"
           << "  elbow position: " << arm.elbowPosition() << "\n";

        // Get fingers
        const Leap::FingerList fingers = hand.fingers();
        for (Leap::FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl)
        {
            const Leap::Finger finger = *fl;
            ss << "  " << fingerNames[finger.type()] << " finger (id: " << finger.id() << ")\n";

            // Get finger bones
            for (int b = 0; b < 4; ++b)
            {
                Leap::Bone::Type boneType = static_cast<Leap::Bone::Type>(b);
                Leap::Bone bone = finger.bone(boneType);
                ss << "    " << boneNames[boneType] << " bone\n"
                   << "      start: " << bone.prevJoint() << "\n"
                   << "      end: " << bone.nextJoint() << "\n"
                   << "      direction: " << bone.direction() << "\n";
            }
        }
    }

    return ss.str();
}

namespace Debug
{
    constexpr float VECTOR_BASIS_LENGTH = 10.0f;

    constexpr Vector3 VECTOR_ORIGIN = {0.0f, 0.0f, 0.0f};
    constexpr Vector3 VECTOR_BASIS_I = {VECTOR_BASIS_LENGTH, 0.0f, 0.0f};
    constexpr Vector3 VECTOR_BASIS_J = {0.0f, VECTOR_BASIS_LENGTH, 0.0f};
    constexpr Vector3 VECTOR_BASIS_K = {0.0f, 0.0f, VECTOR_BASIS_LENGTH};

    constexpr Color VECTOR_BASIS_I_COLOR = RED;
    constexpr Color VECTOR_BASIS_J_COLOR = GREEN;
    constexpr Color VECTOR_BASIS_K_COLOR = BLUE;

    void DrawCartesianBasis()
    {
        DrawLine3D(VECTOR_ORIGIN, VECTOR_BASIS_I, VECTOR_BASIS_I_COLOR);
        DrawLine3D(VECTOR_ORIGIN, VECTOR_BASIS_J, VECTOR_BASIS_J_COLOR);
        DrawLine3D(VECTOR_ORIGIN, VECTOR_BASIS_K, VECTOR_BASIS_K_COLOR);
    }

    constexpr float CAMERA_MOVE_SPEED = 0.09f;
    constexpr float CAMERA_ROTATION_SPEED = 0.03f;

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

        if (IsKeyDown(KEY_Z))
        {
            camera.target = (Vector3){0.0f, 0.0f, 0.0f};
            camera.up = (Vector3){0.0f, 1.0f, 0.0f};
        }
    }

    constexpr int SCREEN_WIDTH = 1600;
    constexpr int SCREEN_HEIGHT = 900;
}

int main()
{
    SampleListener listener;
    Leap::Controller controller;

    controller.addListener(listener);

    InitWindow(Debug::SCREEN_WIDTH, Debug::SCREEN_HEIGHT, "something something idk");

    Camera3D camera{};
    camera.position = (Vector3){10.0f, 10.0f, 10.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        Debug::UpdateCamera(camera);

        Leap::Frame leapFrame = controller.frame();
        std::string frameString = StringifyFrame(leapFrame);

        BeginDrawing();
        {
            ClearBackground(LIGHTGRAY);
            BeginMode3D(camera);
            {
                Debug::DrawCartesianBasis();

                auto hands = leapFrame.hands();
                for (auto it = hands.begin(); it != hands.end(); it++)
                {
                    auto hand = *it;
                    Color col = hand.isLeft() ? ORANGE : VIOLET;
                    auto palmNormal = Helpers::Vec3LeapToRaylib(hand.palmNormal());
                    DrawLine3D(Debug::VECTOR_ORIGIN, palmNormal, col);
                }
            }
            EndMode3D();

            DrawText(frameString.c_str(), 10, 10, 10, BLACK);
        }
        EndDrawing();
    }

    CloseWindow();
    controller.removeListener(listener);
    return 0;
}