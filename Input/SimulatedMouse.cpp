#include "SimulatedMouse.hpp"

#include <Windows.h>

#include <cstring>
#include <iostream>

namespace Input::Mouse
{

namespace
{
enum class Coordinate
{
    X,
    Y
};

template <Coordinate coord>
int ToNormalizedAbsoluteCoordinates(int pixels)
{
    static_assert(coord == Coordinate::X || coord == Coordinate::Y);

    int metricIndex;
    if constexpr (coord == Coordinate::X)
        metricIndex = SM_CXSCREEN;
    else if constexpr (coord == Coordinate::Y)
        metricIndex = SM_CYSCREEN;
    
    float scaleFactor = 65536.0f / GetSystemMetrics(metricIndex);
    return static_cast<int>(scaleFactor * pixels);
}
}  // namespace

// TODO: does not seem to work...
void MoveRelative(int x, int y)
{
    INPUT input;
    std::memset(&input, 0, sizeof(INPUT));
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    input.mi.dx = ToNormalizedAbsoluteCoordinates<Coordinate::X>(x);
    input.mi.dy = ToNormalizedAbsoluteCoordinates<Coordinate::Y>(y);
    SendInput(1, &input, sizeof(INPUT));
}

void MoveAbsolute(int x, int y)
{
    INPUT input;
    std::memset(&input, 0, sizeof(INPUT));
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
    input.mi.dx = ToNormalizedAbsoluteCoordinates<Coordinate::X>(x);
    input.mi.dy = ToNormalizedAbsoluteCoordinates<Coordinate::Y>(y);
    SendInput(1, &input, sizeof(INPUT));
}

void LeftClick()
{
    INPUT input;
    std::memset(&input, 0, sizeof(INPUT));
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

}  // namespace Input::Mouse