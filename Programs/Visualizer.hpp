#pragma once

#include <atomic>
#include <string>
#include <vector>
#include <LeapC.h>

namespace Visualization
{

struct Renderables
{
    std::string leapDebugString;
    std::vector<LEAP_HAND> hands;
    float averageFingerDirectionX;
    float averageFingerDirectionY;
    float cursorDirectionX;
    float cursorDirectionY;
};

void RenderLoop(Renderables& renderables, std::atomic<bool>& isRendering);

}