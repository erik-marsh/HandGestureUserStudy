#include "HttpServer.hpp"

#include <httplib.h>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/schema.h>

#include <HTML/HTMLTemplate.hpp>
#include <Helpers/JSONEvents.hpp>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>

#include "Logging.hpp"

namespace Http
{

using Req = httplib::Request;
using Res = httplib::Response;

auto parseErrorHandler = [](const Req& req, Res& res, Helpers::ParseError error)
{
    switch (error)
    {
        case Helpers::ParseError::SchemaNotValidJSON:
            std::cout << "Schema was not valid." << std::endl;
            res.status = 500;  // 500 Internal Server Error
            break;
        case Helpers::ParseError::RequestNotValidJSON:
            std::cout << "Request was not valid JSON." << std::endl;
            res.status = 400;  // 400 Bad Request
            break;
        case Helpers::ParseError::RequestDoesNotFollowSchema:
            std::cout << "Request did not adhere to schema." << std::endl;
            res.status = 400;  // 400 Bad Request
            break;
        default:
            std::cout << "Parse failed but there was no error?" << std::endl;
            res.status = 500;  // 500 Internal Server Error
            break;
    }
};

auto printRequest = [](const Req& req, Res& res)
{
    std::stringstream ss;
    ss << "[" << req.path << "] " << req.body << "\n";
    std::cout << ss.str();
};

enum class InputDevice
{
    Mouse,
    LeapMotion
};

enum class Task
{
    Form,
    Email
};

constexpr std::array<Task, 2> TASK_SEQUENCE = {Task::Form, Task::Email};

// 2x2 latin square
constexpr std::array<std::array<InputDevice, 2>, 2> COUNTERBALANCING_SEQUENCE = {
    {{InputDevice::Mouse, InputDevice::LeapMotion}, {InputDevice::LeapMotion, InputDevice::Mouse}}};

struct StudyState
{
    bool isStudyDone;
    int userId;
    int counterbalancingIndex;
    int currentTaskIndex;
    int currentDeviceIndex;
};

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
        messageQueue.push(newMessage);
        queueSize = messageQueue.size();
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
    std::atomic<int> queueSize{0};
    std::atomic<bool> programTerminated{false};
};

Expected<int, Helpers::ParseError> ParseInt(const std::string& value, int min, int max)
{
    // TODO: constexpr initialization doesn't work lol
    static const Expected<int, Helpers::ParseError> err(
        Helpers::ParseError::None);  // TODO: better error

    try
    {
        int val = std::stoi(value);
        if (val < min || val > max)
            return err;

        return Expected<int, Helpers::ParseError>(std::move(val));  // TODO: ugly
    }
    catch (const std::exception& ex)
    {
        return err;
    }
}

std::mutex heartbeatMutex;
std::condition_variable heartbeatCV;
std::atomic<bool> killHeartbeat{false};

void HeartbeatLoop(std::atomic<bool>& isRunning, EventDispatcher& dispatcher,
                   const int intervalSeconds)
{
    std::cout << "Starting SSE heartbeat thread..." << std::endl;
    while (isRunning)
    {
        dispatcher.SendEvent("event: keep-alive\ndata: null\n\n");
        // std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
        std::unique_lock<std::mutex> lock(heartbeatMutex);
        heartbeatCV.wait_for(lock, std::chrono::seconds(intervalSeconds),
                             [] { return killHeartbeat.load(); });
    }
}

void HttpServerLoop(std::atomic<bool>& isRunning, std::atomic<bool>& isLeapDriverActive)
{
    std::cout << "Starting HTTP thread..." << std::endl;

    httplib::Server server;
    StudyState state{};
    HTML::HTMLTemplate formTemplate("HTMLTemplates/formTemplate.html");
    HTML::HTMLTemplate emailTemplate("HTMLTemplates/emailTemplate.html");

    if (!server.set_mount_point("/", "./www/"))
    {
        std::cout << "Unable to set mount point" << std::endl;
        isRunning.store(false);
        return;
    }

    EventDispatcher dispatcher;
    auto r_isRunning = std::ref(isRunning);
    auto r_dispatcher = std::ref(dispatcher);
    std::thread heartbeatThread(HeartbeatLoop, r_isRunning, r_dispatcher, 10);

    server.Post("/eventPusherTester",
                [&dispatcher](const Req& req, Res& res)
                {
                    std::cout << "sending event..." << std::endl;
                    dispatcher.SendEvent("event: proceed\ndata: test from eventPusherTester\n\n");
                });

    server.Get("/eventPusher",
               [&dispatcher](const Req& req, Res& res)
               {
                   std::cout << "Got request for eventPusher" << std::endl;
                   res.set_chunked_content_provider(
                       "text/event-stream",
                       [&dispatcher](size_t offset, httplib::DataSink& sink)
                       {
                           dispatcher.WaitEvent(sink);
                           return true;
                       });
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

    auto startHandler = [&state, &isLeapDriverActive](const Req& req, Res& res)
    {
        auto result = Helpers::ParseRequest<Helpers::Start>(req.body);
        if (result.HasValue())
        {
            state.userId = result.Value().userId;
            state.counterbalancingIndex = state.userId % 2;

            // TODO: need to test this to make sure the device (de)activation works
            auto deviceSequence = COUNTERBALANCING_SEQUENCE[state.counterbalancingIndex];
            auto activeDevice = deviceSequence[0];
            isLeapDriverActive.store(activeDevice == InputDevice::LeapMotion);

            std::cout << "Starting user study for user id " << state.userId
                      << " (counterbalancing index=" << state.counterbalancingIndex << ")"
                      << std::endl;
            res.status = 200;  // 200 OK
            return;
        }
        parseErrorHandler(req, res, result.Error());
    };
    server.Post("/start", startHandler);

    auto formHandler = [&state, &formTemplate, &emailTemplate](const Req& req, Res& res)
    {
        int currDevice = -1;
        int currTask = -1;
        for (auto it = req.params.begin(); it != req.params.end(); ++it)
        {
            if (it->first == "currDevice")
            {
                auto result = ParseInt(it->second, 0, COUNTERBALANCING_SEQUENCE[0].size() - 1);
                if (!result.HasValue())
                {
                    res.status = 400;
                    return;
                }
                currDevice = result.Value();
            }

            if (it->first == "currTask")
            {
                auto result = ParseInt(it->second, 0, TASK_SEQUENCE.size() - 1);
                if (!result.HasValue())
                {
                    res.status = 400;
                    return;
                }
                currTask = result.Value();
            }
        }

        if (currDevice == -1 || currTask == -1)
        {
            res.status = 400;
            return;
        }

        std::cout << "success, got device=" << currDevice << " and task=" << currTask << std::endl;
        switch (TASK_SEQUENCE[currTask])
        {
            case Task::Form:
                formTemplate.Substitute({"Jeremiah the Bullfrog", "jbf@gmail.com",
                                         "123 Swamp Apt. 227", "4/16/1954", "12345678",
                                         "123-45-678"});
                res.set_content(formTemplate.GetSubstitution(), "text/html");
                break;
            case Task::Email:
                emailTemplate.Substitute(
                    {"jbf@gmail.com", "hey dude what's up it's me, [REDACTED]"});
                res.set_content(emailTemplate.GetSubstitution(), "text/html");
                break;
            default:
                std::cout << "Something went horribly wrong" << std::endl;
                res.status = 500;
                break;
        }
    };
    server.Get("/form", formHandler);

    // logging only
    auto eventsClickHandler = [](const Req& req, Res& res)
    {
        auto result = Helpers::ParseRequest<Helpers::EventClick>(req.body);
        if (result.HasValue())
        {
            printRequest(req, res);
            res.status = 200;
            return;
        }
        parseErrorHandler(req, res, result.Error());
    };
    server.Post("/events/click", eventsClickHandler);

    // logging only
    auto eventsKeystrokeHandler = [](const Req& req, Res& res)
    {
        auto result = Helpers::ParseRequest<Helpers::EventKeystroke>(req.body);
        if (result.HasValue())
        {
            printRequest(req, res);
            res.status = 200;
            return;
        }
        parseErrorHandler(req, res, result.Error());
    };
    server.Post("/events/keystroke", eventsKeystrokeHandler);

    // logging only, relevant state is client-side only
    auto eventsFieldHandler = [](const Req& req, Res& res)
    {
        auto result = Helpers::ParseRequest<Helpers::EventFieldCompletion>(req.body);
        if (result.HasValue())
        {
            printRequest(req, res);
            res.status = 200;
            return;
        }
        parseErrorHandler(req, res, result.Error());
    };
    server.Post("/events/field", eventsFieldHandler);

    // we actually need to do state updates here
    auto eventsTaskHandler = [&state, &isLeapDriverActive](const Req& req, Res& res)
    {
        auto result = Helpers::ParseRequest<Helpers::EventTaskCompletion>(req.body);
        if (result.HasValue())
        {
            state.currentTaskIndex++;
            if (state.currentTaskIndex == TASK_SEQUENCE.size())
            {
                state.currentTaskIndex = 0;
                state.currentDeviceIndex++;
            }

            if (state.currentDeviceIndex ==
                COUNTERBALANCING_SEQUENCE[state.counterbalancingIndex].size())
            {
                state.isStudyDone = true;
            }

            // TODO: need to test this to make sure the device (de)activation works
            auto deviceSequence = COUNTERBALANCING_SEQUENCE[state.counterbalancingIndex];
            auto activeDevice = deviceSequence[0];
            isLeapDriverActive.store(activeDevice == InputDevice::LeapMotion);

            std::stringstream ss;
            ss << "Study state:"
               << "\n    Done?: " << std::boolalpha << state.isStudyDone
               << "\n    userId: " << state.userId
               << "\n    counterbalancingIndex: " << state.counterbalancingIndex
               << "\n    currentTaskIndex: " << state.currentTaskIndex
               << "\n    currentDeviceIndex: " << state.currentDeviceIndex << std::endl;
            std::cout << ss.str();

            printRequest(req, res);
            res.status = 200;
            return;
        }
        parseErrorHandler(req, res, result.Error());
    };
    server.Post("/events/task", eventsTaskHandler);

    server.listen("localhost", 5000);
    std::cout << "server.listen exited" << std::endl;
    heartbeatThread.join();  // TODO: note that the sleep_for is still running

    std::cout << "Shutting down HTTP thread..." << std::endl;
}

}  // namespace Http