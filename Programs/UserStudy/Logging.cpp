#include "Logging.hpp"

#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace Logging
{

// this is incredibly stupid
// this is to correctly manage state lifetime in order to write to the file at program exit
class LoggerState
{
   public:
    LoggerState() {}
    ~LoggerState()
    {
        auto openMode = isFileInitialized ? std::ios::app : std::ios::trunc;
        // currIndex points to the next line to write, not the most recent line
        std::ofstream outFile(logFilename, openMode);
        for (int i = 0; i < currIndex; i++)
            outFile << logDoubleBuffer[currBuffer][i] << "\n";
        std::cout << "Wrote log file to " << logFilename << "." << std::endl;
    }

    const std::string logFilename = "eventLog.csv";
    const std::string delimiter = ";";

    std::array<std::array<std::string, 1000>, 2> logDoubleBuffer;
    int currBuffer = 0;
    int currIndex = 0;
    bool isFileInitialized = false;
};

LoggerState state;

// helper functions
template <Loggable T>
consteval std::string_view EventTypeToString();

template <Loggable T>
std::string SerializeEvent(T event);

std::string ClickLocationToString(Events::ClickLocation loc);

// implementations
template <Loggable T>
void Log(T event)
{
    std::string logLine = SerializeEvent(event);

    // the buffer should not be full at this point
    auto& buffer = state.logDoubleBuffer[state.currBuffer];
    buffer[state.currIndex] = logLine;

    state.currIndex++;
    if (state.currIndex == buffer.size())
    {
        // buffer is now semantically the backbuffer
        state.currBuffer = (state.currBuffer + 1) % 2;
        state.currIndex = 0;

        // write the backbuffer
        auto openMode = state.isFileInitialized ? std::ios::app : std::ios::trunc;
        std::ofstream outFile(state.logFilename, openMode);
        for (auto it = buffer.begin(); it != buffer.end(); ++it)
            outFile << *it << "\n";
    }
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
    ss << std::boolalpha << EventTypeToString<Events::Click>() << state.delimiter
       << event.timestampMillis << state.delimiter << ClickLocationToString(event.location)
       << state.delimiter << event.wasCorrect;
    return ss.str();
}

template <>
std::string SerializeEvent(Events::CursorPosition event)
{
    std::stringstream ss;
    ss << std::boolalpha << EventTypeToString<Events::CursorPosition>() << state.delimiter
       << event.timestampMillis << state.delimiter << event.positionX << state.delimiter
       << event.positionY;
    return ss.str();
}

template <>
std::string SerializeEvent(Events::Keystroke event)
{
    std::stringstream ss;
    ss << std::boolalpha << EventTypeToString<Events::Keystroke>() << state.delimiter
       << event.timestampMillis << state.delimiter << event.keycode << state.delimiter
       << event.wasCorrect;
    return ss.str();
}

template <>
std::string SerializeEvent(Events::FieldCompletion event)
{
    std::stringstream ss;
    ss << std::boolalpha << EventTypeToString<Events::FieldCompletion>() << state.delimiter
       << event.timestampMillis << state.delimiter << event.fieldIndex << state.delimiter
       << event.totalFields;
    return ss.str();
}

template <>
std::string SerializeEvent(Events::TaskCompletion event)
{
    std::stringstream ss;
    ss << std::boolalpha << EventTypeToString<Events::TaskCompletion>() << state.delimiter
       << event.timestampMillis << state.delimiter << event.taskIndex << state.delimiter
       << event.taskName << state.delimiter << event.totalTasks;
    return ss.str();
}

}  // namespace Logging