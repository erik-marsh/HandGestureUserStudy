#pragma once

namespace Input
{
class SimulatedMouse
{
   public:
    SimulatedMouse();
    ~SimulatedMouse();

    void Warp(const int x, const int y) const;
    void MoveRelative(const int x, const int y) const;
};
}  // namespace Input