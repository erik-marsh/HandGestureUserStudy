#pragma once

#include <LeapC.h>

#include <atomic>
#include <mutex>
#include <string>
#include <vector>

namespace Visualization
{

struct Renderables
{
    bool hasHand;
    LEAP_HAND hand;
    bool didClick;
    float avgFingerDirX;
    float avgFingerDirY;
    float cursorDirX;
    float cursorDirY;
};

void RenderLoop(Renderables& renderables, std::atomic<bool>& isRunning,
                std::mutex& renderableCopyMutex);

}  // namespace Visualization