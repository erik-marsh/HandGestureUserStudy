#pragma once

#include <Helpers/IsAnyOf.hpp>
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

}  // namespace Events


///////////////////////////////////////////////////////////////////////////////
// Logger constants and etc
///////////////////////////////////////////////////////////////////////////////
constexpr char DELIMITER = ';';

template <typename T>
concept Loggable = IsAnyOf<T, Events::Click, Events::CursorPosition, Events::Keystroke,
                           Events::FieldCompletion, Events::TaskCompletion>;

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