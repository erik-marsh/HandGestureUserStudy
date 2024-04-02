#pragma once

#include <Helpers/IsAnyOf.hpp>
#include <array>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <mutex>
#include <string>
#include <type_traits>

namespace Logging
{

///////////////////////////////////////////////////////////////////////////////
// Event structures
///////////////////////////////////////////////////////////////////////////////
namespace Events
{

enum class ClickLocation
{
    OutOfBounds,  // TODO: this might not be necessary, and is for sure impossible to catch in JS
    Background,
    TextField,
    Button
};

struct Click
{
    uint64_t timestampMillis;
    ClickLocation location;
    bool wasCorrect;
};

struct CursorPosition
{
    uint64_t timestampMillis;
    int positionX;
    int positionY;
};

struct Keystroke
{
    uint64_t timestampMillis;
    std::string key;
    bool wasCorrect;
};

struct FieldCompletion
{
    uint64_t timestampMillis;
    int fieldIndex;
};

struct TaskCompletion
{
    uint64_t timestampMillis;
    int taskIndex;
};

struct DeviceChanged
{
    uint64_t timestampMillis;
    std::string newDevice;
};

}  // namespace Events

///////////////////////////////////////////////////////////////////////////////
// Logger constants and etc
///////////////////////////////////////////////////////////////////////////////
constexpr char DELIMITER = ';';

template <typename T>
concept Loggable = IsAnyOf<T, Events::Click, Events::CursorPosition, Events::Keystroke,
                           Events::FieldCompletion, Events::TaskCompletion, Events::DeviceChanged>;

uint64_t GetCurrentUnixTimeMillis();

///////////////////////////////////////////////////////////////////////////////
// Logger class declaration
///////////////////////////////////////////////////////////////////////////////
class Logger
{
   public:
    Logger();
    Logger(std::string filename);
    ~Logger();

    void OpenLogFile(const std::string& filename);

    template <Loggable T>
    void Log(T event);

   private:
    std::string logFilename;
    bool hasFilename;

    std::array<std::array<std::string, 1000>, 2> logDoubleBuffer;
    int currBuffer;
    int currIndex;
    bool isFileInitialized;
    std::mutex mutex;
};

template <Loggable T>
std::string SerializeEvent(T event);

template <Loggable T>
void Logger::Log(T event)
{
    std::lock_guard<std::mutex> lock(mutex);

    assert(hasFilename);
    std::string logLine = SerializeEvent(event);

    // the buffer should not be full at this point
    auto& buffer = logDoubleBuffer[currBuffer];
    buffer[currIndex] = logLine;

    currIndex++;
    if (currIndex == buffer.size())
    {
        // buffer is now semantically the backbuffer
        currBuffer = (currBuffer + 1) % 2;
        currIndex = 0;

        // write the backbuffer
        auto openMode = isFileInitialized ? std::ios::app : std::ios::trunc;
        std::ofstream outFile(logFilename, openMode);
        for (auto it = buffer.begin(); it != buffer.end(); ++it)
            outFile << *it << "\n";

        if (!isFileInitialized)
            isFileInitialized = true;
    }
}

}  // namespace Logging