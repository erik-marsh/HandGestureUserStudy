#pragma once

#include <atomic>

namespace Http
{

void HttpServerLoop(std::atomic<bool>& isRunning);

}