#pragma once

#include <Programs/UserStudy/Logging.hpp>
#include <array>

namespace Helpers
{

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

constexpr int NUM_TASKS = 2;
constexpr int NUM_DEVICES = 2;
constexpr std::array<Task, NUM_TASKS> TASK_SEQUENCE = {Task::Form, Task::Email};
// 2x2 latin square
constexpr std::array<std::array<InputDevice, NUM_DEVICES>, 2> COUNTERBALANCING_SEQUENCE = {
    {{InputDevice::Mouse, InputDevice::LeapMotion}, {InputDevice::LeapMotion, InputDevice::Mouse}}};

class StudyStateMachine
{
   public:
    enum class State
    {
        Start,
        Tutorial,
        PracticeTask,
        Task,
        End
    };

    State GetState() const { return currState; }
    int GetDeviceIndex() const { return currDeviceIndex; }
    int GetTaskIndex() const { return currTaskIndex; }
    int GetCounterbalancingIndex() const { return counterbalancingIndex; }
    int GetUserId() const { return userId; }

    Task GetCurrTask() const { return TASK_SEQUENCE[currTaskIndex]; }
    InputDevice GetCurrInputDevice() const
    {
        return COUNTERBALANCING_SEQUENCE[counterbalancingIndex][currDeviceIndex];
    }


    void InitializeUser(int userId)
    {
        this->userId = userId;
        counterbalancingIndex = userId % 2;
    }

    void Proceed()
    {
        switch (currState)
        {
            case State::Start:
            {
                currState = State::Tutorial;
                break;
            }
            case State::Tutorial:
            {
                // TODO: implement practice tasks
                currState = State::Task;
                currDeviceIndex = 0;
                currTaskIndex = 0;
                break;
            }
            case State::PracticeTask:
            {
                // TODO: see above
                break;
            }
            case State::Task:
            {
                const auto& cbSeq = COUNTERBALANCING_SEQUENCE[counterbalancingIndex];

                currTaskIndex++;
                if (currTaskIndex == TASK_SEQUENCE.size())
                {
                    currTaskIndex = 0;
                    currDeviceIndex++;
                }
                if (currDeviceIndex == cbSeq.size())
                {
                    currTaskIndex = 0;
                    currDeviceIndex = 0;
                    currState = State::End;
                }
                break;
            }
            case State::End:
                break;  // no-op
        }
    }

   private:
    int userId;
    int counterbalancingIndex;

    State currState = State::Start;
    int currDeviceIndex = 0;
    int currTaskIndex = 0;
};

}  // namespace Helpers