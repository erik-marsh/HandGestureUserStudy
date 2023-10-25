#include "HttpServer.hpp"

#include <httplib.h>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/schema.h>

#include <HTML/HTMLTemplate.hpp>
#include <Helpers/HTTPHelpers.hpp>
#include <Helpers/JSONEvents.hpp>
#include <Helpers/SSE.hpp>
#include <Helpers/StringPools.hpp>
#include <Helpers/StudyData.hpp>
#include <Helpers/UserIDLock.hpp>
#include <Input/SimulatedMouse.hpp>
#include <filesystem>
#include <format>
#include <iostream>
#include <sstream>
#include <string>

#include "Logging.hpp"

namespace Http
{

using Req = httplib::Request;
using Res = httplib::Response;

const std::string LOG_BASE_DIR = "Logs";

void HttpServerLoop(SyncState& syncState)
{
    std::cout << "[main] Starting HTTP thread...\n";

    if (!std::filesystem::exists(LOG_BASE_DIR))
        std::filesystem::create_directory(LOG_BASE_DIR);

    httplib::Server server;
    if (!server.set_mount_point("/", "./www/"))
    {
        std::cout << "[HTTP] Unable to set mount point.\n";
        syncState.isRunning.store(false);
        return;
    }

    // Set up objects for use by the server
    Helpers::StudyStateMachine state;
    Helpers::UserIDLock userIdLock("ids.lock");

    HTML::HTMLTemplate startTemplate("HTMLTemplates/startPage.html");
    HTML::HTMLTemplate tutorialTemplate("HTMLTemplates/tutorialPage.html");
    HTML::HTMLTemplate endTemplate("HTMLTemplates/endPage.html");
    HTML::HTMLTemplate formTemplate("HTMLTemplates/formTemplate.html");
    HTML::HTMLTemplate emailTemplate("HTMLTemplates/emailTemplate.html");

    Helpers::EventDispatcher dispatcher;
    auto r_isRunning = std::ref(syncState.isRunning);
    auto r_dispatcher = std::ref(dispatcher);
    std::thread heartbeatThread(Helpers::HeartbeatLoop, r_isRunning, r_dispatcher, 3);

    // Declare all the lambdas used to service HTTP requests
    auto eventPusherHandler = [&dispatcher](const Req& req, Res& res)
    {
        res.set_chunked_content_provider("text/event-stream",
                                         [&dispatcher](size_t offset, httplib::DataSink& sink)
                                         {
                                             dispatcher.WaitEvent(sink);
                                             return true;
                                         });
    };

    auto quitHandler = [&server, &syncState, &dispatcher](const Req& req, Res& res)
    {
        std::cout << "[HTTP] Server got shutdown signal, shutting down threads...\n";
        dispatcher.ShutDown();
        {
            std::lock_guard<std::mutex> lock(Helpers::heartbeatMutex);
            Helpers::killHeartbeat = true;
            Helpers::heartbeatCV.notify_all();
        }
        server.stop();
        syncState.isRunning.store(false);
    };

    auto startHandler = [&state, &syncState, &dispatcher, &userIdLock](const Req& req, Res& res)
    {
        using enum Helpers::StudyStateMachine::State;

        if (state.GetState() != Start)
        {
            res.status = 400;
            return;
        }

        auto result = Helpers::ParseRequest<Helpers::Start>(req.body);
        if (result.HasValue())
        {
            int userId = result.Value().userId;

            // check if the user ID has been used already
            if (userIdLock.IsLocked(userId))
            {
                std::cout << std::format("[HTTP] User ID {} is already in use.\n", userId);
                res.status = 403;
                return;
            }

            state.InitializeUser(userId);
            state.Proceed();

            userIdLock.Lock(state.GetUserId());
            std::string logFilename = std::format("{}/user{}.log", LOG_BASE_DIR, userId);
            syncState.logger.OpenLogFile(logFilename);
            syncState.isLogging = true;

            std::cout << std::format(
                "[HTTP] Starting user study for user ID {} (counterbalancing index={}).\n", userId,
                state.GetCounterbalancingIndex());
            dispatcher.SendEvent("event: proceed\r\ndata: starting tutorial\r\n\r\n");

            res.status = 200;  // 200 OK
            return;
        }

        Helpers::parseErrorHandler(req, res, result.Error());
    };

    auto tutorialProgressHandler = [&state, &dispatcher](const Req& req, Res& res)
    {
        using enum Helpers::StudyStateMachine::State;

        if (state.GetState() != Tutorial)
        {
            res.status = 400;
            return;
        }

        state.Proceed();
        dispatcher.SendEvent("event: proceed\r\ndata: starting user study\r\n\r\n");

        res.status = 200;
    };

    auto formTestHandler = [&formTemplate, &emailTemplate, &tutorialTemplate, &startTemplate,
                            &endTemplate](const Req& req, Res& res)
    {
        if (!req.has_param("target"))
        {
            res.status = 404;
            return;
        }

        auto target = req.get_param_value("target");
        if (target == "start")
        {
            res.set_content(startTemplate.GetSubstitution(), "text/html");
        }
        else if (target == "tutorial")
        {
            res.set_content(tutorialTemplate.GetSubstitution(), "text/html");
        }
        else if (target == "end")
        {
            res.set_content(endTemplate.GetSubstitution(), "text/html");
        }
        else if (target == "form")
        {
            using namespace Helpers::StringPools;
            std::vector<std::string> strings;
            strings.push_back("NO_DEVICE");
            strings.push_back("NO_DEVICE");
            strings.push_back("0");
            strings.push_back("0");
            strings.push_back(SelectRandom(Names));
            strings.push_back(SelectRandom(EmailAddresses));
            strings.push_back(SelectRandom(PhysicalAddresses));
            strings.push_back(SelectRandom(DateOfBirth));
            strings.push_back(SelectRandom(IdNumbers));
            strings.push_back(SelectRandom(CardNumbers));
            formTemplate.Substitute(strings);
            res.set_content(formTemplate.GetSubstitution(), "text/html");
        }
        else if (target == "email")
        {
            using namespace Helpers::StringPools;
            std::vector<std::string> strings;
            strings.push_back("NO_DEVICE");
            strings.push_back("0");
            strings.push_back("0");
            strings.push_back(SelectRandom(EmailAddresses));
            strings.push_back(SelectRandom(EmailBodyText));
            emailTemplate.Substitute(strings);
            res.set_content(emailTemplate.GetSubstitution(), "text/html");
        }
        else
        {
            res.status = 404;
        }
    };

    auto formHandler = [&state, &formTemplate, &emailTemplate, &tutorialTemplate, &startTemplate,
                        &endTemplate, &syncState](const Req& req, Res& res)
    {
        using namespace Helpers::StringPools;
        using enum Helpers::StudyStateMachine::State;
        using enum Helpers::InputDevice;
        using enum Helpers::Task;

        if (state.GetState() == Start)
        {
            res.set_content(startTemplate.GetSubstitution(), "text/html");
            return;
        }

        if (state.GetState() == Tutorial)
        {
            res.set_content(tutorialTemplate.GetSubstitution(), "text/html");
            syncState.isLeapDriverActive.store(true);
            return;
        }

        if (state.GetState() == End)
        {
            res.set_content(endTemplate.GetSubstitution(), "text/html");
            return;
        }

        // valid states from here on out: {PracticeTask, Task}
        // we want to reset mouse position before each task
        Input::Mouse::MoveAbsolute(100, 100);

        std::string device;
        switch (state.GetCurrInputDevice())
        {
            case Mouse:
                device = "Mouse";
                syncState.isLeapDriverActive.store(false);
                break;
            case LeapMotion:
                device = "Leap Motion";
                syncState.isLeapDriverActive.store(true);
                break;
            default:
                device = "<UNKNOWN>";
                syncState.isLeapDriverActive.store(false);
                break;
        }

        std::vector<std::string> strings;
        strings.push_back(device);
        strings.push_back(std::to_string(state.GetTaskIndex() + 1));
        strings.push_back(std::to_string(Helpers::TASK_SEQUENCE.size()));

        switch (state.GetCurrTask())
        {
            case Form:
                strings.emplace(strings.begin(), device);
                strings.push_back(SelectRandom(Names));
                strings.push_back(SelectRandom(EmailAddresses));
                strings.push_back(SelectRandom(PhysicalAddresses));
                strings.push_back(SelectRandom(DateOfBirth));
                strings.push_back(SelectRandom(IdNumbers));
                strings.push_back(SelectRandom(CardNumbers));
                formTemplate.Substitute(strings);
                res.set_content(formTemplate.GetSubstitution(), "text/html");
                break;
            case Email:
                strings.push_back(SelectRandom(EmailAddresses));
                strings.push_back(SelectRandom(EmailBodyText));
                emailTemplate.Substitute(strings);
                res.set_content(emailTemplate.GetSubstitution(), "text/html");
                break;
            default:
                std::cout << "[HTTP] Something went horribly wrong...\n";
                res.status = 500;
                break;
        }
    };

    // logging only
    auto eventsClickHandler = [&syncState](const Req& req, Res& res)
    {
        auto result = Helpers::ParseRequest<Helpers::EventClick>(req.body);
        if (result.HasValue())
        {
            for (auto event : result.Value().data)
                syncState.logger.Log(event);
            Helpers::printRequest(req, res);
            res.status = 200;
            return;
        }
        Helpers::parseErrorHandler(req, res, result.Error());
    };

    // logging only
    auto eventsKeystrokeHandler = [&syncState](const Req& req, Res& res)
    {
        auto result = Helpers::ParseRequest<Helpers::EventKeystroke>(req.body);
        if (result.HasValue())
        {
            for (auto event : result.Value().data)
                syncState.logger.Log(event);
            Helpers::printRequest(req, res);
            res.status = 200;
            return;
        }
        Helpers::parseErrorHandler(req, res, result.Error());
    };

    // logging only, relevant state is client-side only
    auto eventsFieldHandler = [&syncState](const Req& req, Res& res)
    {
        auto result = Helpers::ParseRequest<Helpers::EventFieldCompletion>(req.body);
        if (result.HasValue())
        {
            syncState.logger.Log(result.Value().data);
            Helpers::printRequest(req, res);
            res.status = 200;
            return;
        }
        Helpers::parseErrorHandler(req, res, result.Error());
    };

    auto eventsTaskHandler = [&state, &syncState, &dispatcher](const Req& req, Res& res)
    {
        using enum Helpers::StudyStateMachine::State;

        if (state.GetState() == End)
        {
            res.status = 400;
            return;
        }

        auto result = Helpers::ParseRequest<Helpers::EventTaskCompletion>(req.body);
        if (result.HasValue())
        {
            using enum Helpers::StudyStateMachine::State;

            syncState.logger.Log(result.Value().data);
            state.Proceed();

            std::cout << std::format(
                "[HTTP] Study state"
                "\n    Done?: {}"
                "\n    userId: {}"
                "\n    counterbalancingIndex: {}"
                "\n    currentTaskIndex: {}"
                "\n    currentDeviceIndex: {}\n",
                state.GetState() == End, state.GetUserId(), state.GetCounterbalancingIndex(),
                state.GetTaskIndex(), state.GetDeviceIndex());

            if (state.GetState() == End)
            {
                res.status = 200;
                dispatcher.SendEvent("event: proceed\r\ndata: study is done\r\n\r\n");
                return;
            }

            dispatcher.SendEvent("event: proceed\r\ndata: moving on to next study phase\r\n\r\n");

            Helpers::printRequest(req, res);
            res.status = 200;
            return;
        }

        Helpers::parseErrorHandler(req, res, result.Error());
    };

    // Hook up the lambdas to the server and begin listening
    server.set_error_handler(Helpers::errorHandler);
    server.set_exception_handler(Helpers::exceptionHandler);
    server.set_pre_routing_handler(
        [](const Req& req, Res& res)
        {
            std::cout << std::format("[HTTP] {} {}\n", req.method, req.path);
            return httplib::Server::HandlerResponse::Unhandled;
        });

    server.Get("/", formHandler);
    server.Get("/test", formTestHandler);
    server.Get("/eventPusher", eventPusherHandler);

    server.Post("/quit", quitHandler);
    server.Post("/start", startHandler);                           // TODO: merge these
    server.Post("/acknowledgeTutorial", tutorialProgressHandler);  // TODO: merge these
    server.Post("/events/click", eventsClickHandler);
    server.Post("/events/keystroke", eventsKeystrokeHandler);
    server.Post("/events/field", eventsFieldHandler);
    server.Post("/events/task", eventsTaskHandler);

    server.listen("localhost", 5000);
    heartbeatThread.join();

    std::cout << "[main] Shutting down HTTP thread...\n";
}

}  // namespace Http