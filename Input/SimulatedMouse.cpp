#include "SimulatedMouse.hpp"

namespace Input
{

SimulatedMouse::SimulatedMouse() : m_xdoContext(xdo_new(nullptr)) {}

SimulatedMouse::~SimulatedMouse() { xdo_free(m_xdoContext); }

void SimulatedMouse::Warp(const int x, const int y) const
{
    xdo_move_mouse_relative(m_xdoContext, x, y);
}

void SimulatedMouse::MoveRelative(const int x, const int y) const
{
    xdo_move_mouse_relative(m_xdoContext, x, y);
}

}  // namespace Input