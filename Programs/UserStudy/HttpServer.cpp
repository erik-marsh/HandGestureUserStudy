#include "HttpServer.hpp"

#include <httplib.h>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/schema.h>

#include <HTML/HTMLTemplate.hpp>
#include <Helpers/JSONEvents.hpp>
#include <Helpers/StringPools.hpp>
#include <Helpers/UserIDLock.hpp>
#include <exception>
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
    bool isInTutorial;
    bool isStudyStarted;
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
        dispatcher.SendEvent("event: keep-alive\ndata: null\r\n\r\n");
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
    HTML::HTMLTemplate startTemplate("HTMLTemplates/startPage.html");
    HTML::HTMLTemplate endTemplate("HTMLTemplates/endPage.html");
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
    // std::thread heartbeatThread(HeartbeatLoop, r_isRunning, r_dispatcher, 10);

    server.Post(
        "/eventPusherTester",
        [&dispatcher](const Req& req, Res& res)
        {
            std::cout << "sending event..." << std::endl;
            dispatcher.SendEvent("event: proceed\ndata: test from eventPusherTester\r\n\r\n");
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

    Helpers::UserIDLock userIdLock("ids.lock");

    auto startHandler =
        [&state, &isLeapDriverActive, &dispatcher, &userIdLock](const Req& req, Res& res)
    {
        if (state.isStudyStarted)
        {
            res.status = 400;
            return;
        }

        auto result = Helpers::ParseRequest<Helpers::Start>(req.body);
        if (result.HasValue())
        {
            // check if the user ID has been used already
            if (userIdLock.IsLocked(result.Value().userId))
            {
                std::cout << "User ID " << result.Value().userId << " is already in use.";
                res.status = 403;
                return;
            }

            state.isStudyStarted = true;
            state.userId = result.Value().userId;
            userIdLock.Lock(state.userId);
            state.counterbalancingIndex = state.userId % 2;

            // TODO: need to test this to make sure the device (de)activation works
            auto deviceSequence = COUNTERBALANCING_SEQUENCE[state.counterbalancingIndex];
            auto activeDevice = deviceSequence[0];
            isLeapDriverActive.store(activeDevice == InputDevice::LeapMotion);

            std::cout << "Starting user study for user id " << state.userId
                      << " (counterbalancing index=" << state.counterbalancingIndex << ")"
                      << std::endl;

            dispatcher.SendEvent("event: proceed\ndata: starting user study\r\n\r\n");

            res.status = 200;  // 200 OK
            return;
        }
        parseErrorHandler(req, res, result.Error());
    };
    server.Post("/start", startHandler);

    auto formHandler = [&state, &formTemplate, &emailTemplate, &startTemplate, &endTemplate](
                           const Req& req, Res& res)
    {
        if (!state.isStudyStarted)
        {
            res.set_content(startTemplate.GetSubstitution(), "text/html");
            return;
        }

        if (state.isStudyDone)
        {
            res.set_content(endTemplate.GetSubstitution(), "text/html");
            return;
        }

        // int currDevice = -1;
        // int currTask = -1;
        // for (auto it = req.params.begin(); it != req.params.end(); ++it)
        // {
        //     if (it->first == "currDevice")
        //     {
        //         auto result = ParseInt(it->second, 0, COUNTERBALANCING_SEQUENCE[0].size() - 1);
        //         if (!result.HasValue())
        //         {
        //             res.status = 400;
        //             return;
        //         }
        //         currDevice = result.Value();
        //     }

        //     if (it->first == "currTask")
        //     {
        //         auto result = ParseInt(it->second, 0, TASK_SEQUENCE.size() - 1);
        //         if (!result.HasValue())
        //         {
        //             res.status = 400;
        //             return;
        //         }
        //         currTask = result.Value();
        //     }
        // }

        // if (currDevice == -1 || currTask == -1)
        // {
        //     res.status = 400;
        //     return;
        // }

        // std::cout << "success, got device=" << currDevice << " and task=" << currTask <<
        // std::endl;

        std::string device;
        switch (COUNTERBALANCING_SEQUENCE[state.counterbalancingIndex][state.currentDeviceIndex])
        {
            case InputDevice::Mouse:
                device = "Mouse";
                break;
            case InputDevice::LeapMotion:
                device = "Leap Motion";
                break;
            default:
                device = "<UNKNOWN>";
                break;
        }

        std::vector<std::string> strings;
        strings.push_back(device);
        strings.push_back(std::to_string(state.currentTaskIndex + 1));
        strings.push_back(std::to_string(TASK_SEQUENCE.size()));
        using namespace Helpers::StringPools;
        switch (TASK_SEQUENCE[state.currentTaskIndex])
        {
            case Task::Form:
                strings.push_back(SelectRandom(Names));
                strings.push_back(SelectRandom(EmailAddresses));
                strings.push_back(SelectRandom(PhysicalAddresses));
                strings.push_back(SelectRandom(DateOfBirth));
                strings.push_back(SelectRandom(IdNumbers));
                strings.push_back(SelectRandom(CardNumbers));
                formTemplate.Substitute(strings);
                res.set_content(formTemplate.GetSubstitution(), "text/html");
                break;
            case Task::Email:
                strings.push_back(SelectRandom(EmailAddresses));
                strings.push_back(SelectRandom(EmailBodyText));
                emailTemplate.Substitute(strings);
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
    auto eventsTaskHandler = [&state, &isLeapDriverActive, &dispatcher](const Req& req, Res& res)
    {
        if (state.isStudyDone)
        {
            res.status = 400;
            return;
        }

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
                res.status = 200;
                dispatcher.SendEvent("event: proceed\ndata: study is done\r\n\r\n");
                return;
            }

            // TODO: need to test this to make sure the device (de)activation works
            auto deviceSequence = COUNTERBALANCING_SEQUENCE[state.counterbalancingIndex];
            auto activeDevice = deviceSequence[state.currentDeviceIndex];
            isLeapDriverActive.store(activeDevice == InputDevice::LeapMotion);
            std::cout << "LEAP MOTION ACTIVE?: " << isLeapDriverActive << std::endl;

            dispatcher.SendEvent("event: proceed\ndata: moving on to next study phase\r\n\r\n");

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

    server.set_error_handler(
        [](const Req& req, Res& res)
        {
            std::stringstream ss;
            ss << "<h1>Error " << res.status << "/<h1>";
            res.set_content(ss.str(), "text/html");
        });

    server.set_exception_handler(
        [](const Req& req, Res& res, std::exception_ptr exp)
        {
            std::stringstream ss;
            ss << "<h1>Error 500</h1><p>";
            try
            {
                std::rethrow_exception(exp);
            }
            catch (std::exception& ex)
            {
                ss << ex.what();
            }
            catch (...)
            {
                ss << "Unknown exception (not of type std::exception) was thrown.";
            }
            ss << "</p>";

            res.set_content(ss.str(), "text/html");
        });

    server.listen("localhost", 5000);
    std::cout << "server.listen exited" << std::endl;
    // heartbeatThread.join();  // TODO: note that the sleep_for is still running

    std::cout << "Shutting down HTTP thread..." << std::endl;
}

}  // namespace Http