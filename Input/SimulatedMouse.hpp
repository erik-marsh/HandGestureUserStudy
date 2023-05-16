#pragma once

#include <utility>  // std::pair

namespace Input::Mouse
{

void MoveRelative(int x, int y);
void MoveAbsolute(int x, int y);
void LeftClick();
std::pair<int, int> QueryMousePosition();

}  // namespace Input::Mouse