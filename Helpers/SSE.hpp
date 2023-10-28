#pragma once

#include <httplib.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string_view>

namespace Helpers
{

class EventDispatcher
{
   public:
    void WaitEvent(httplib::DataSink& sink);
    void SendEvent(std::string_view newMessage);
    void ShutDown();

   private:
    std::mutex queueMutex;
    std::condition_variable cv;
    std::queue<std::string> messageQueue;
    std::atomic<int> eventId{0};
    std::atomic<int> queueSize{0};
    std::atomic<bool> programTerminated{false};
};

void HeartbeatLoop(std::atomic<bool>& isRunning, Helpers::EventDispatcher& dispatcher,
                   const int intervalSeconds);

namespace
{

std::mutex heartbeatMutex;
std::condition_variable heartbeatCV;
std::atomic<bool> killHeartbeat{false};

}  // namespace

}  // namespace Helpers