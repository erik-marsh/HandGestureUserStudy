#pragma once

#include "../xdotool/xdo.h"

namespace Input
{
class SimulatedMouse
{
   public:
    SimulatedMouse();
    ~SimulatedMouse();

    void Warp(const int x, const int y) const;
    void MoveRelative(const int x, const int y) const;

   private:
    xdo_t* const m_xdoContext;
};
}  // namespace Input