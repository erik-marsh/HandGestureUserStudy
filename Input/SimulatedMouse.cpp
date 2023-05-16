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

// Note: In practice, the dx and dy are approximate
// If you pass in (10, 10), you won't necessarily get a differential of (10, 10),
// but it will be pretty close (+/-3).
void MoveRelative(int x, int y)
{
    INPUT input;
    std::memset(&input, 0, sizeof(INPUT));
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    input.mi.dx = x;
    input.mi.dy = y;
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

std::pair<int, int> QueryMousePosition()
{
    POINT point;
    GetCursorPos(&point);
    return {point.x, point.y};
}

}  // namespace Input::Mouse