#pragma once

#include <Input/LeapConnection.hpp>
#include <atomic>

#include "Visualizer.hpp"

namespace Input
{

void DriverLoop(Leap::LeapConnection& connection, Visualization::Renderables& renderables,
                std::atomic<bool>& isRunning, std::atomic<bool>& isLeapDriverActive,
                std::mutex& renderableCopyMutex);

}