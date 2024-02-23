#include "CursorLogger.hpp"

#include <Input/SimulatedMouse.hpp>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <Windows.h>

#include "Logging.hpp"

namespace Logging
{

uint64_t GetCurrentUnixTimeMillis()
{
    // Number of milliseconds between the "Windows epoch"
    // (Jan 1, 1601 00:00) and the Unix epoch (Jan 1, 1970 00:00).
    static constexpr uint64_t windowsEpochToUnixEpochMillis = 11644473600000;

    FILETIME filetime;
    GetSystemTimeAsFileTime(&filetime);

    ULARGE_INTEGER qwFiletime;
    qwFiletime.LowPart = filetime.dwLowDateTime;
    qwFiletime.HighPart = filetime.dwHighDateTime;

    const uint64_t timeSince1601_100ns = static_cast<uint64_t>(qwFiletime.QuadPart);
    const uint64_t timeSince1601_ms = timeSince1601_100ns / 10000;
    return timeSince1601_ms - windowsEpochToUnixEpochMillis;
}

void CursorLoggerLoop(SyncState& syncState)
{
    std::cout << "[main] Starting cursor position logging thread...\n";

    using Clock = std::chrono::high_resolution_clock;
    using Time = std::chrono::high_resolution_clock::time_point;
    using Nanos = std::chrono::nanoseconds;
    using Millis = std::chrono::milliseconds;

    constexpr Nanos loopTime = Nanos(33'333'333);  // 33.33ms aka 30Hz

    while (syncState.isRunning.load())
    {
        // TODO: don't spinlock lol
        while (!syncState.isLogging.load())
            ;

        Time loopStart = Clock::now();
        uint64_t timestamp = GetCurrentUnixTimeMillis();

        auto posResult = Input::Mouse::QueryMousePosition();
        Logging::Events::CursorPosition pos{};
        pos.timestampMillis = timestamp;
        pos.positionX = posResult.first;
        pos.positionY = posResult.second;
        syncState.logger.Log(pos);

        Time loopEnd = Clock::now();
        Nanos processingTime = loopEnd - loopStart;
        if (processingTime.count() > 0)
            std::this_thread::sleep_for(loopTime - processingTime);
    }

    std::cout << "[main] Shutting down cursor position logging thread...\n";
}

}  // namespace Logging