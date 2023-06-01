#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <type_traits>

namespace Logging
{

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

template <typename T>
concept Loggable = std::same_as<Events::Click, T> || std::same_as<Events::CursorPosition, T> ||
                   std::same_as<Events::Keystroke, T> || std::same_as<Events::FieldCompletion, T> ||
                   std::same_as<Events::TaskCompletion, T>;

template <Loggable T>
void Log(T event);

// forward template declarations
template void Log(Events::Click);
template void Log(Events::CursorPosition);
template void Log(Events::Keystroke);
template void Log(Events::FieldCompletion);
template void Log(Events::TaskCompletion);

}  // namespace Logging