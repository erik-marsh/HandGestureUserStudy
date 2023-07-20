#pragma once

#include <httplib.h>
#include <Helpers/JSONEvents.hpp>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <sstream>

namespace Helpers
{

namespace
{

class EventDispatcher
{
   public:
    void WaitEvent(httplib::DataSink& sink)
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [this] { return queueSize > 0 || programTerminated; });
        // TODO: there's still probably a race condition i didn't account for
        // lock is reacquired...
        if (programTerminated)
            return;

        std::string message = messageQueue.front();
        sink.write(message.c_str(), message.length());
        messageQueue.pop();
        queueSize = messageQueue.size();
        // lock is released
    }

    void SendEvent(const std::string& newMessage)
    {
        if (programTerminated)
            return;

        std::lock_guard<std::mutex> lock(queueMutex);
        std::stringstream ss;
        ss << "id: " << eventId.load() << "\n" << newMessage;
        messageQueue.push(ss.str());
        queueSize = messageQueue.size();
        eventId++;
        cv.notify_all();
    }

    void ShutDown()
    {
        programTerminated = true;
        cv.notify_all();
    }

   private:
    std::mutex queueMutex;
    std::condition_variable cv;
    std::queue<std::string> messageQueue;
    std::atomic<int> eventId{0};
    std::atomic<int> queueSize{0};
    std::atomic<bool> programTerminated{false};
};

std::mutex heartbeatMutex;
std::condition_variable heartbeatCV;
std::atomic<bool> killHeartbeat{false};

void HeartbeatLoop(std::atomic<bool>& isRunning, Helpers::EventDispatcher& dispatcher,
                   const int intervalSeconds)
{
    std::cout << "Starting SSE heartbeat thread..." << std::endl;
    while (isRunning)
    {
        dispatcher.SendEvent("data: keep alive\r\n\r\n");
        std::unique_lock<std::mutex> lock(heartbeatMutex);
        heartbeatCV.wait_for(lock, std::chrono::seconds(intervalSeconds),
                             [] { return killHeartbeat.load(); });
    }
}

}

}  // namespace Helpers