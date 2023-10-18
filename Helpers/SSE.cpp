#include "SSE.hpp"

#include <iostream>
#include <sstream>

namespace Helpers
{

void EventDispatcher::WaitEvent(httplib::DataSink& sink)
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

void EventDispatcher::SendEvent(const std::string& newMessage)
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

void EventDispatcher::ShutDown()
{
    programTerminated = true;
    cv.notify_all();
}

void HeartbeatLoop(std::atomic<bool>& isRunning, EventDispatcher& dispatcher,
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

}  // namespace Helpers