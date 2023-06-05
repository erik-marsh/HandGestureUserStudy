#pragma once

#include <array>
#include <cstdint>
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
    OutOfBounds,
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
    char keycode;
    bool wasCorrect;
};

struct FieldCompletion
{
    uint64_t timestampMillis;
    int fieldIndex;
    int totalFields;  // TODO: unnecessary? (i remember this being a time-crunch hack)
};

struct TaskCompletion
{
    uint64_t timestampMillis;
    int taskIndex;
    std::string taskName;  // TODO: unnecessary?
    int totalTasks;        // TODO: unnecessary?
};

}  // namespace Events


///////////////////////////////////////////////////////////////////////////////
// Logger constants and etc
///////////////////////////////////////////////////////////////////////////////
constexpr char DELIMITER = ';';

template <typename T>
concept Loggable = std::same_as<Events::Click, T> || std::same_as<Events::CursorPosition, T> ||
                   std::same_as<Events::Keystroke, T> || std::same_as<Events::FieldCompletion, T> ||
                   std::same_as<Events::TaskCompletion, T>;

///////////////////////////////////////////////////////////////////////////////
// Logger class declaration
///////////////////////////////////////////////////////////////////////////////
class Logger
{
   public:
    Logger(std::string filename);
    ~Logger();

    template <Loggable T>
    void Log(T event);

   private:
    const std::string logFilename;

    std::array<std::array<std::string, 1000>, 2> logDoubleBuffer;
    int currBuffer;
    int currIndex;
    bool isFileInitialized;
};

///////////////////////////////////////////////////////////////////////////////
// Forward template declarations
///////////////////////////////////////////////////////////////////////////////
template void Logger::Log(Events::Click);
template void Logger::Log(Events::CursorPosition);
template void Logger::Log(Events::Keystroke);
template void Logger::Log(Events::FieldCompletion);
template void Logger::Log(Events::TaskCompletion);

}  // namespace Logging