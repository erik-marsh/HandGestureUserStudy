#include "Logging.hpp"

#include <array>
#include <format>
#include <iostream>
#include <sstream>
#include <string>
#include <Windows.h>

namespace Logging
{

///////////////////////////////////////////////////////////////////////////////
// Forward declarations for helper functions
///////////////////////////////////////////////////////////////////////////////
template <Loggable T>
consteval std::string_view EventTypeToString();

std::string ClickLocationToString(Events::ClickLocation loc);

///////////////////////////////////////////////////////////////////////////////
// Implementations of class methods
///////////////////////////////////////////////////////////////////////////////
Logger::Logger() : hasFilename(false), currBuffer(0), currIndex(0), isFileInitialized(false) {}

Logger::Logger(std::string filename)
    : logFilename(filename),
      hasFilename(true),
      currBuffer(0),
      currIndex(0),
      isFileInitialized(false)
{
}

Logger::~Logger()
{
    auto openMode = isFileInitialized ? std::ios::app : std::ios::trunc;
    // currIndex points to the next line to write, not the most recent line
    std::ofstream outFile(logFilename, openMode);
    for (int i = 0; i < currIndex; i++)
        outFile << logDoubleBuffer[currBuffer][i] << "\n";

    std::cout << std::format("[Event Logging] Closing log file {} and writing it to disk.\n",
                             logFilename);
}

void Logger::OpenLogFile(const std::string& filename)
{
    logFilename = filename;
    hasFilename = true;
}

///////////////////////////////////////////////////////////////////////////////
// Implementations of helper functions
///////////////////////////////////////////////////////////////////////////////
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

template <Loggable T>
consteval std::string_view EventTypeToString()
{
    if constexpr (std::is_same_v<T, Events::Click>)
        return "Click";
    else if constexpr (std::is_same_v<T, Events::CursorPosition>)
        return "CursorPosition";
    else if constexpr (std::is_same_v<T, Events::Keystroke>)
        return "Keystroke";
    else if constexpr (std::is_same_v<T, Events::FieldCompletion>)
        return "FieldCompletion";
    else if constexpr (std::is_same_v<T, Events::TaskCompletion>)
        return "TaskCompletion";
    else if constexpr (std::is_same_v<T, Events::DeviceChanged>)
        return "DeviceChanged";
}

std::string ClickLocationToString(Events::ClickLocation loc)
{
    switch (loc)
    {
        case Events::ClickLocation::OutOfBounds:
            return "OutOfBounds";
        case Events::ClickLocation::Background:
            return "Background";
        case Events::ClickLocation::TextField:
            return "TextField";
        case Events::ClickLocation::Button:
            return "Button";
        default:
            return "<unknown>";
    }
}

template <>
std::string SerializeEvent(Events::Click event)
{
    std::stringstream ss;
    ss << std::boolalpha << EventTypeToString<Events::Click>() << DELIMITER << event.timestampMillis
       << DELIMITER << ClickLocationToString(event.location) << DELIMITER << event.wasCorrect;
    return ss.str();
}

template <>
std::string SerializeEvent(Events::CursorPosition event)
{
    std::stringstream ss;
    ss << std::boolalpha << EventTypeToString<Events::CursorPosition>() << DELIMITER
       << event.timestampMillis << DELIMITER << event.positionX << DELIMITER << event.positionY;
    return ss.str();
}

template <>
std::string SerializeEvent(Events::Keystroke event)
{
    std::stringstream ss;
    ss << std::boolalpha << EventTypeToString<Events::Keystroke>() << DELIMITER
       << event.timestampMillis << DELIMITER << event.key << DELIMITER << event.wasCorrect;
    return ss.str();
}

template <>
std::string SerializeEvent(Events::FieldCompletion event)
{
    std::stringstream ss;
    ss << std::boolalpha << EventTypeToString<Events::FieldCompletion>() << DELIMITER
       << event.timestampMillis << DELIMITER << event.fieldIndex;
    return ss.str();
}

template <>
std::string SerializeEvent(Events::TaskCompletion event)
{
    std::stringstream ss;
    ss << std::boolalpha << EventTypeToString<Events::TaskCompletion>() << DELIMITER
       << event.timestampMillis << DELIMITER << event.taskIndex;
    return ss.str();
}

template <>
std::string SerializeEvent(Events::DeviceChanged event)
{
    std::stringstream ss;
    ss << std::boolalpha << EventTypeToString<Events::DeviceChanged>() << DELIMITER
       << event.timestampMillis << DELIMITER << event.newDevice;
    return ss.str();
}

}  // namespace Logging