#include "LeapDebug.hpp"

#include <sstream>
#include <string>

namespace Debug
{

std::string StringifyFrame(LEAP_TRACKING_EVENT& frame)
{
    std::stringstream ss;
    ss << "Frame id: " << frame.tracking_frame_id << "\nTimestamp: " << frame.info.timestamp
       << "\nHands: " << frame.nHands << "\n";

    auto hands = frame.pHands;
    for (int i = 0; i < frame.nHands; i++)
    {
        auto hand = frame.pHands[i];

        std::string handType =
            hand.type == eLeapHandType::eLeapHandType_Left ? "Left hand" : "Right hand";
        const LEAP_VECTOR normal = hand.palm.normal;
        const LEAP_VECTOR direction = hand.palm.direction;

        ss << handType << " (id: " << hand.id << ")\n";
    }

    return ss.str();
}
}  // namespace Debug