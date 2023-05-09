#pragma once

#include <array>
#include <string>

#include "../LeapSDK/include/LeapC.h"

namespace Debug
{

const std::array<std::string, 5> fingerNames = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const std::array<std::string, 4> boneNames = {"Metacarpal", "Proximal", "Middle", "Distal"};
const std::array<std::string, 4> stateNames = {"STATE_INVALID", "STATE_START", "STATE_UPDATE",
                                               "STATE_END"};

std::string StringifyFrame(LEAP_TRACKING_EVENT& frame);

}  // namespace Debug