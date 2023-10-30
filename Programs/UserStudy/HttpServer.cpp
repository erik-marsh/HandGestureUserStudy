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

constexpr std::string_view LOG_BASE_DIR = "Logs";
constexpr std::string_view SSE_PROCEED_MESSAGE = "event: proceed\r\ndata: null\r\n\r\n";

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
    Helpers::StudyStateMachine studyControl;
    Helpers::UserIDLock userIdLock("ids.lock");

    HTML::HTMLTemplate startTemplate("HTMLTemplates/startPage.html");
    HTML::HTMLTemplate tutorialTemplate("HTMLTemplates/instructionsPage.html");
    HTML::HTMLTemplate endTemplate("HTMLTemplates/endPage.html");
    HTML::HTMLTemplate postTutorialTemplate("HTMLTemplates/postTutorial.html");
    HTML::HTMLTemplate formTemplate("HTMLTemplates/formTemplate.html");
    HTML::HTMLTemplate formTemplateTutorial("HTMLTemplates/formTemplateTutorial.html");
    HTML::HTMLTemplate emailTemplate("HTMLTemplates/emailTemplate.html");
    HTML::HTMLTemplate emailTemplateTutorial("HTMLTemplates/emailTemplateTutorial.html");

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

    auto startHandler =
        [&studyControl, &syncState, &dispatcher, &userIdLock](const Req& req, Res& res)
    {
        using enum Helpers::StudyStateMachine::State;

        if (studyControl.GetState() != Start)
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

            studyControl.InitializeUser(userId);
            studyControl.Proceed();

            userIdLock.Lock(studyControl.GetUserId());
            std::string logFilename = std::format("{}/user{}.log", LOG_BASE_DIR, userId);
            syncState.logger.OpenLogFile(logFilename);
            syncState.isLogging = true;

            std::cout << std::format(
                "[HTTP] Starting user study for user ID {} (counterbalancing index={}).\n", userId,
                studyControl.GetCounterbalancingIndex());
            dispatcher.SendEvent("event: proceed\r\ndata: starting tutorial\r\n\r\n");

            res.status = 200;  // 200 OK
            return;
        }

        Helpers::parseErrorHandler(req, res, result.Error());
    };

    auto proceedHandler = [&studyControl, &dispatcher](const Req& req, Res& res)
    {
        using enum Helpers::StudyStateMachine::State;

        // proceeding from Start is directly handled by the user ID submission handler
        // this is because the user id needs to be initialized in order for
        // studyControl to be properly initialized
        if (studyControl.GetState() == Start)
        {
            res.status = 400;
            return;
        }

        // TODO: return 428 Precondition Required if certain data has not been submitted yet
        studyControl.Proceed();                     // advance the server's internal state
        dispatcher.SendEvent(SSE_PROCEED_MESSAGE);  // tell the client to refresh the page

        std::cout << std::format(
            "[HTTP] Study studyControl"
            "\n    Done?: {}"
            "\n    userId: {}"
            "\n    counterbalancingIndex: {}"
            "\n    currentTaskIndex: {}"
            "\n    currentDeviceIndex: {}\n",
            studyControl.GetState() == End, studyControl.GetUserId(),
            studyControl.GetCounterbalancingIndex(), studyControl.GetTaskIndex(),
            studyControl.GetDeviceIndex());
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

    auto pageHandler = [&studyControl, &formTemplate, &formTemplateTutorial, &emailTemplate,
                        &emailTemplateTutorial, &tutorialTemplate, &postTutorialTemplate,
                        &startTemplate, &endTemplate, &syncState](const Req& req, Res& res)
    {
        using namespace Helpers::StringPools;
        using enum Helpers::StudyStateMachine::State;
        using enum Helpers::InputDevice;
        using enum Helpers::Task;

        if (studyControl.GetState() == Start)
        {
            res.set_content(startTemplate.GetSubstitution(), "text/html");
            return;
        }

        if (studyControl.GetState() == Instructions)
        {
            res.set_content(tutorialTemplate.GetSubstitution(), "text/html");
            syncState.isLeapDriverActive.store(true);
            return;
        }

        if (studyControl.GetState() == End)
        {
            res.set_content(endTemplate.GetSubstitution(), "text/html");
            return;
        }

        if (studyControl.GetState() == PostTutorial)
        {
            res.set_content(postTutorialTemplate.GetSubstitution(), "text/html");
            return;
        }

        // valid states from here on out: {TutorialTask, Task}
        // we want to reset mouse position before each task
        Input::Mouse::MoveAbsolute(100, 100);

        std::string device;
        switch (studyControl.GetCurrInputDevice())
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
        strings.push_back(std::to_string(studyControl.GetTaskIndex() + 1));
        strings.push_back(std::to_string(Helpers::TASK_SEQUENCE.size()));

        switch (studyControl.GetCurrTask())
        {
            case Form:
            {
                strings.emplace(strings.begin(), device);
                strings.push_back(SelectRandom(Names));
                strings.push_back(SelectRandom(EmailAddresses));
                strings.push_back(SelectRandom(PhysicalAddresses));
                strings.push_back(SelectRandom(DateOfBirth));
                strings.push_back(SelectRandom(IdNumbers));
                strings.push_back(SelectRandom(CardNumbers));

                if (studyControl.GetState() == TutorialTask)
                {
                    formTemplateTutorial.Substitute(strings);
                    res.set_content(formTemplateTutorial.GetSubstitution(), "text/html");
                }
                else if (studyControl.GetState() == Task)
                {
                    formTemplate.Substitute(strings);
                    res.set_content(formTemplate.GetSubstitution(), "text/html");
                }
                break;
            }
            case Email:
            {
                strings.push_back(SelectRandom(EmailAddresses));
                strings.push_back(SelectRandom(EmailBodyText));

                if (studyControl.GetState() == TutorialTask)
                {
                    emailTemplateTutorial.Substitute(strings);
                    res.set_content(emailTemplateTutorial.GetSubstitution(), "text/html");
                }
                else if (studyControl.GetState() == Task)
                {
                    emailTemplate.Substitute(strings);
                    res.set_content(emailTemplate.GetSubstitution(), "text/html");
                }
                break;
            }
            default:
            {
                std::cout << "[HTTP] Something went horribly wrong...\n";
                res.status = 500;
                break;
            }
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
            res.status = 200;
            return;
        }
        Helpers::parseErrorHandler(req, res, result.Error());
    };

    auto eventsTaskHandler = [&studyControl, &syncState, &dispatcher](const Req& req, Res& res)
    {
        auto result = Helpers::ParseRequest<Helpers::EventTaskCompletion>(req.body);
        if (!result.HasValue())
        {
            syncState.logger.Log(result.Value().data);
            res.status = 200;
            return;
        }
        Helpers::parseErrorHandler(req, res, result.Error());
    };

    // Hook up the lambdas to the server and begin listening
    server.set_error_handler(Helpers::errorHandler);
    server.set_exception_handler(Helpers::exceptionHandler);
    server.set_post_routing_handler(Helpers::postRoutingDebugPrint);

    server.Get("/", pageHandler);
    server.Get("/eventPusher", eventPusherHandler);

    server.Post("/start", startHandler);
    server.Post("/proceed", proceedHandler);
    // TODO: now that the above handler exists, we can implement a proper tutorial
    //       the things that it needs are as follows:
    //        1. templates that mirror the actual tasks,
    //           but have a notice that they are tutorial tasks
    //        2. modified JS that does not submit events (as not to log stuff)
    //        3. State machine logic and HTTP handler updates to accomodate this
    //        4. a post-tutorial page that congratulates the user and tells them
    //           that they will be doing the same thing but for real this time
    server.Post("/quit", quitHandler);

    server.Post("/events/click", eventsClickHandler);
    server.Post("/events/keystroke", eventsKeystrokeHandler);
    server.Post("/events/field", eventsFieldHandler);
    server.Post("/events/task", eventsTaskHandler);

    server.listen("localhost", 5000);
    heartbeatThread.join();

    std::cout << "[main] Shutting down HTTP thread...\n";
}

}  // namespace Http