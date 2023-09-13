#pragma once

#include <Input/LeapConnection.hpp>
#include <atomic>
#include <mutex>

#include "Logging.hpp"

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

struct SyncState
{
    SyncState() = delete;
    SyncState(Input::Leap::LeapConnection& conn, Renderables& rend, std::mutex& rcm,
              std::atomic<bool>& running, std::atomic<bool>& leapActive, std::atomic<bool>& logging)
        : connection(conn),
          renderables(rend),
          renderableCopyMutex(rcm),
          isRunning(running),
          isLeapDriverActive(leapActive),
          isLogging(logging)
    {
    }

    SyncState(const SyncState&) = delete;
    SyncState(const SyncState&&) = delete;

    Logging::Logger logger;
    Input::Leap::LeapConnection& connection;
    Renderables& renderables;
    std::mutex& renderableCopyMutex;
    std::atomic<bool>& isRunning;
    std::atomic<bool>& isLeapDriverActive;
    std::atomic<bool>& isLogging;
};