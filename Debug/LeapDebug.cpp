#include "LeapDebug.hpp"

namespace Debug
{
const std::array<std::string, 5> EventListener::fingerNames = {"Thumb", "Index", "Middle", "Ring",
                                                               "Pinky"};
const std::array<std::string, 4> EventListener::boneNames = {"Metacarpal", "Proximal", "Middle",
                                                             "Distal"};
const std::array<std::string, 4> EventListener::stateNames = {"STATE_INVALID", "STATE_START",
                                                              "STATE_UPDATE", "STATE_END"};

void EventListener::onInit(const Leap::Controller& controller)
{
    std::cout << "[LeapMotion Controller] Initialized" << std::endl;
}

void EventListener::onConnect(const Leap::Controller& controller)
{
    std::cout << "[LeapMotion Controller] Connected" << std::endl;
}

void EventListener::onDisconnect(const Leap::Controller& controller)
{
    // Note: not dispatched when running in a debugger.
    std::cout << "[LeapMotion Controller] Disconnected" << std::endl;
}

void EventListener::onExit(const Leap::Controller& controller)
{
    std::cout << "[LeapMotion Controller] Exited" << std::endl;
}

void EventListener::onFrame(const Leap::Controller& controller) {}

void EventListener::onFocusGained(const Leap::Controller& controller)
{
    std::cout << "[LeapMotion Controller] Focus Gained" << std::endl;
}

void EventListener::onFocusLost(const Leap::Controller& controller)
{
    std::cout << "[LeapMotion Controller] Focus Lost" << std::endl;
}

void EventListener::onDeviceChange(const Leap::Controller& controller)
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

void EventListener::onServiceConnect(const Leap::Controller& controller)
{
    std::cout << "[LeapMotion Controller] Service Connected" << std::endl;
}

void EventListener::onServiceDisconnect(const Leap::Controller& controller)
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
            ss << "  " << EventListener::fingerNames[finger.type()]
               << " finger (id: " << finger.id() << ")\n";

            // Get finger bones
            for (int b = 0; b < 4; ++b)
            {
                Leap::Bone::Type boneType = static_cast<Leap::Bone::Type>(b);
                Leap::Bone bone = finger.bone(boneType);
                ss << "    " << EventListener::boneNames[boneType] << " bone\n"
                   << "      start: " << bone.prevJoint() << "\n"
                   << "      end: " << bone.nextJoint() << "\n"
                   << "      direction: " << bone.direction() << "\n";
            }
        }
    }

    return ss.str();
}
}