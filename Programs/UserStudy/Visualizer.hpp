#pragma once

#include <LeapC.h>

#include <atomic>
#include <string>
#include <vector>

namespace Visualization
{

struct Renderables
{
    LEAP_HAND hand;
    bool didClick;
    float avgFingerDirX;
    float avgFingerDirY;
    float cursorDirX;
    float cursorDirY;
};

void RenderLoop(Renderables& renderables, std::atomic<bool>& isRunning);

}  // namespace Visualization