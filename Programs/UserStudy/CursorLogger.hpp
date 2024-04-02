#pragma once

#include "SyncState.hpp"

namespace Logging
{

uint64_t GetCurrentUnixTimeMillis();
void CursorLoggerLoop(SyncState& syncState);

}