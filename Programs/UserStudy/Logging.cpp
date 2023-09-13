#include "Logging.hpp"

#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cassert>

namespace Logging
{

///////////////////////////////////////////////////////////////////////////////
// Forward declarations for helper functions
///////////////////////////////////////////////////////////////////////////////
template <Loggable T>
consteval std::string_view EventTypeToString();

template <Loggable T>
std::string SerializeEvent(T event);

std::string ClickLocationToString(Events::ClickLocation loc);

///////////////////////////////////////////////////////////////////////////////
// Implementations of class methods
///////////////////////////////////////////////////////////////////////////////
Logger::Logger() : hasFilename(false), currBuffer(0), currIndex(0), isFileInitialized(false) {}

Logger::Logger(std::string filename)
    : logFilename(filename), hasFilename(true), currBuffer(0), currIndex(0), isFileInitialized(false)
{
}

Logger::~Logger()
{
    auto openMode = isFileInitialized ? std::ios::app : std::ios::trunc;
    // currIndex points to the next line to write, not the most recent line
    std::ofstream outFile(logFilename, openMode);
    for (int i = 0; i < currIndex; i++)
        outFile << logDoubleBuffer[currBuffer][i] << "\n";
    std::cout << "Closing log file " << logFilename << " and writing it to disk." << std::endl;
}

void Logger::OpenLogFile(const std::string& filename)
{
    logFilename = filename;
    hasFilename = true;
}

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

///////////////////////////////////////////////////////////////////////////////
// Implementations of helper functions
///////////////////////////////////////////////////////////////////////////////
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

}  // namespace Logging