#include "LeapDebug.hpp"

#include <sstream>
#include <string>

namespace Debug
{
const std::array<std::string, 5> EventListener::fingerNames = {"Thumb", "Index", "Middle", "Ring",
                                                               "Pinky"};
const std::array<std::string, 4> EventListener::boneNames = {"Metacarpal", "Proximal", "Middle",
                                                             "Distal"};
const std::array<std::string, 4> EventListener::stateNames = {"STATE_INVALID", "STATE_START",
                                                              "STATE_UPDATE", "STATE_END"};

std::string StringifyFrame(LEAP_TRACKING_EVENT& frame)
{
    std::stringstream ss;
    ss << "Frame id: " << frame.tracking_frame_id << "\nTimestamp: " << frame.info.timestamp
       << "\nHands: " << frame.nHands
       << /*"\nExtended fingers: " << frame.fingers().extended().count() <<*/ "\n";

    auto hands = frame.pHands;
    for (int i = 0; i < frame.nHands; i++)
    {
        auto hand = frame.pHands[i];

        std::string handType = hand.type == eLeapHandType::eLeapHandType_Left ? "Left hand" : "Right hand";
        const LEAP_VECTOR normal = hand.palm.normal;
        const LEAP_VECTOR direction = hand.palm.direction;

        ss << handType << " (id: " << hand.id << ")\n";
         //   << "  palm position: " << hand.palm.position << "\n"
         //   << "  palm normal: " << normal << "\n"
         //   << "  hand direction: " << direction << "\n";

        // Calculate the hand's pitch, roll, and yaw angles
      //   ss << "  pitch: " << direction.pitch() * Leap::RAD_TO_DEG << " degrees\n"
      //      << "  roll: " << normal.roll() * Leap::RAD_TO_DEG << " degrees\n"
      //      << "  yaw: " << direction.yaw() * Leap::RAD_TO_DEG << " degrees\n";

        // Get the Arm bone
      //   Leap::Arm arm = hand.arm();
      //   ss << "  Arm direction: " << arm.direction() << "\n"
      //      << "  wrist position: " << arm.wristPosition() << "\n"
      //      << "  elbow position: " << arm.elbowPosition() << "\n";

        // Get fingers
      //   const Leap::FingerList fingers = hand.fingers();
      //   for (Leap::FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl)
      //   {
      //       const Leap::Finger finger = *fl;
      //       ss << "  " << EventListener::fingerNames[finger.type()]
      //          << " finger (id: " << finger.id() << ")\n";

      //       // Get finger bones
      //       for (int b = 0; b < 4; ++b)
      //       {
      //           Leap::Bone::Type boneType = static_cast<Leap::Bone::Type>(b);
      //           Leap::Bone bone = finger.bone(boneType);
      //           ss << "    " << EventListener::boneNames[boneType] << " bone\n"
      //              << "      start: " << bone.prevJoint() << "\n"
      //              << "      end: " << bone.nextJoint() << "\n"
      //              << "      direction: " << bone.direction() << "\n";
      //       }
      //   }
    }

    return ss.str();
}
}