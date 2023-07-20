#pragma once

#include <Programs/UserStudy/Logging.hpp>
#include <array>

namespace Helpers
{

struct StudyState
{
    bool isStudyStarted;
    bool isTutorialDone;
    bool isStudyDone;
    Logging::Logger logger;
    int userId;
    int counterbalancingIndex;
    int currentTaskIndex;
    int currentDeviceIndex;
};

enum class InputDevice
{
    Mouse,
    LeapMotion
};

enum class Task
{
    Form,
    Email
};

constexpr std::array<Task, 2> TASK_SEQUENCE = {Task::Form, Task::Email};

// 2x2 latin square
constexpr std::array<std::array<InputDevice, 2>, 2> COUNTERBALANCING_SEQUENCE = {
    {{InputDevice::Mouse, InputDevice::LeapMotion}, {InputDevice::LeapMotion, InputDevice::Mouse}}};

}  // namespace Helpers