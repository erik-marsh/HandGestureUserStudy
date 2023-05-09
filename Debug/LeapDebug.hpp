#pragma once

#include <array>
#include <string>

#include "../LeapSDK/include/LeapC.h"

namespace Debug
{

class EventListener //: public Leap::Listener
{
   public:
    // virtual void onInit(const Leap::Controller&);
    // virtual void onConnect(const Leap::Controller&);
    // virtual void onDisconnect(const Leap::Controller&);
    // virtual void onExit(const Leap::Controller&);
    // virtual void onFrame(const Leap::Controller&);
    // virtual void onFocusGained(const Leap::Controller&);
    // virtual void onFocusLost(const Leap::Controller&);
    // virtual void onDeviceChange(const Leap::Controller&);
    // virtual void onServiceConnect(const Leap::Controller&);
    // virtual void onServiceDisconnect(const Leap::Controller&);

    const static std::array<std::string, 5> fingerNames;
    const static std::array<std::string, 4> boneNames;
    const static std::array<std::string, 4> stateNames;
};

std::string StringifyFrame(LEAP_TRACKING_EVENT& frame);

}