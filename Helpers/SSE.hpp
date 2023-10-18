#pragma once

#include <httplib.h>

#include <Helpers/JSONEvents.hpp>
#include <atomic>
#include <condition_variable>
#include <iostream>
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
        // lock is reacquired...
        if (programTerminated)
            return;

        std::string message = messageQueue.front();

        // if we don't succeed in writing the message, we don't pop it from the queue
        // the next time this function is called, we attempt to write it again.
        // when does WaitEvent get called?
        //     whenever httplib asks the chunked content provider to write something
        // note that the connection is kept alive for some period of time
        //     CPPHTTPLIB defines the keep alive timeout as 5 seconds by default
        //     this value is settable
        //     (we the server push a message to it alive every 3 seconds)
        // while this connection is alive,
        // process_server_socket_core loops until the connection is supposed to close
        //     (it loops at 1000Hz)
        // long story short, at some point during process_server_socket_core,
        // the client is written to again
        bool succeeded = sink.write(message.c_str(), message.length());
        if (succeeded)
            messageQueue.pop();
        queueSize = messageQueue.size();

        // std::stringstream ss;
        // ss << "[SSE] [queue size=" << queueSize << ", write success=" << std::boolalpha << succeeded
        //    << "] Sent event: " << message;
        // std::cout << ss.str();
        
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
    std::cout << "[main] Starting SSE heartbeat thread...\n";
    while (isRunning)
    {
        dispatcher.SendEvent("event: keep-alive\r\ndata: keep-alive\r\n\r\n");
        std::unique_lock<std::mutex> lock(heartbeatMutex);
        heartbeatCV.wait_for(lock, std::chrono::seconds(intervalSeconds),
                             [] { return killHeartbeat.load(); });
    }
    std::cout << "[main] Stopping SSE heartbeat thread...\n";
}

}  // namespace

}  // namespace Helpers