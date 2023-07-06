#include <httplib.h>

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>

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
        std::cout << "Just sent:\n" << message;
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

void HeartbeatLoop(std::atomic<bool>& isRunning, EventDispatcher& dispatcher,
                   const int intervalSeconds)
{
    std::cout << "Starting SSE heartbeat thread..." << std::endl;
    while (isRunning)
    {
        dispatcher.SendEvent("event: qux\ndata: null\r\n\r\n");
        std::unique_lock<std::mutex> lock(heartbeatMutex);
        heartbeatCV.wait_for(lock, std::chrono::seconds(intervalSeconds),
                             [] { return killHeartbeat.load(); });
    }
}

void HeartbeatLoop2(std::atomic<bool>& isRunning, EventDispatcher& dispatcher,
                   const int intervalSeconds)
{
    std::cout << "Starting SSE heartbeat thread..." << std::endl;
    while (isRunning)
    {
        dispatcher.SendEvent("data: keep-alive\r\n\r\n");
        std::unique_lock<std::mutex> lock(heartbeatMutex);
        heartbeatCV.wait_for(lock, std::chrono::seconds(intervalSeconds),
                             [] { return killHeartbeat.load(); });
    }
}

int main()
{
    httplib::Server server;
    EventDispatcher dispatcher;
    std::atomic<bool> isRunning = true;

    auto r_isRunning = std::ref(isRunning);
    auto r_dispatcher = std::ref(dispatcher);
    std::thread heartbeatThread(HeartbeatLoop, r_isRunning, r_dispatcher, 30);
    std::thread heartbeatThread2(HeartbeatLoop2, r_isRunning, r_dispatcher, 3);

    if (!server.set_mount_point("/", "./www/"))
    {
        std::cout << "Unable to set mount point" << std::endl;
        return 1;
    }

    using Req = httplib::Request;
    using Res = httplib::Response;
    server.Get("/eventPusher",
               [&dispatcher](const Req& req, Res& res)
               {
                   std::cout << "Got request for eventPusher" << std::endl;
                   res.status = 200;
                   res.set_header("Connection", "keep-alive");
                   res.set_header("Cache-Control", "no-cache");
                   res.set_chunked_content_provider(
                       "text/event-stream",
                       [&dispatcher](size_t offset, httplib::DataSink& sink)
                       {
                           dispatcher.WaitEvent(sink);
                           return true;
                       });
               });
    
    server.Get("/test", [](const Req& req, Res& res)
    {
        res.set_content(R"(<!DOCTYPE html>
<html lang="en">
<head>
  <script src="sse.js" defer></script>
</head>
<body>
  see console
</body>
</html>)", "text/html");
    });

    auto quitHandler = [&server, &isRunning, &dispatcher](const Req& req, Res& res)
    {
        std::cout << "Server got shutdown signal, shutting down threads..." << std::endl;
        dispatcher.ShutDown();
        {
            std::lock_guard<std::mutex> lock(heartbeatMutex);
            killHeartbeat = true;
            heartbeatCV.notify_all();
        }
        server.stop();
        isRunning.store(false);
    };
    server.Post("/quit", quitHandler);

    server.listen("localhost", 5000);
    heartbeatThread.join();
    heartbeatThread2.join();

    return 0;
}