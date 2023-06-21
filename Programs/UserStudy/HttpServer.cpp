#include "HttpServer.hpp"

#include <httplib.h>
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/schema.h>

#include <HTML/HTMLTemplate.hpp>
#include <Helpers/Expected.hpp>
#include <Helpers/IsAnyOf.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include "Logging.hpp"

namespace Http
{

using Req = httplib::Request;
using Res = httplib::Response;

// https://stackoverflow.com/questions/15911890/overriding-return-type-in-function-template-specialization
// note that the specializations on the struct types are not necessary
// was this a change made in a later c++ standard?
struct RequestData_t
{
};

struct Start : public RequestData_t
{
    int userId;
};

struct EventFieldCompletion : public RequestData_t
{
    Logging::Events::FieldCompletion data;
};

struct EventTaskCompletion : public RequestData_t
{
    Logging::Events::TaskCompletion data;
};

struct EventClick : public RequestData_t
{
    std::vector<Logging::Events::Click> data;
};

struct EventKeystroke : public RequestData_t
{
    std::vector<Logging::Events::Keystroke> data;
};

// I don't exactly understand the rationale,
// but std::derived_from requires that the derived class
// inherit from the base class publically.
template <typename T>
concept RequestData = std::derived_from<T, RequestData_t>;

template <RequestData T>
struct RequestDataReturn_t
{
    using type = T;
};

template <RequestData T>
typename RequestDataReturn_t<T>::type DeserializeRequest(rapidjson::Document& requestDocument);

template <RequestData T>
std::string GetRequestSchema();

enum class ParseError
{
    None = 0,
    SchemaNotValidJSON,
    RequestNotValidJSON,
    RequestDoesNotFollowSchema
};

template <RequestData T>
Expected<T, ParseError> ParseRequest(std::string jsonRequest);

auto parseErrorHandler = [](const Req& req, Res& res, Http::ParseError error)
{
    switch (error)
    {
        case ParseError::SchemaNotValidJSON:
            std::cout << "Schema was not valid." << std::endl;
            res.status = 500;  // 500 Internal Server Error
            break;
        case ParseError::RequestNotValidJSON:
            std::cout << "Request was not valid JSON." << std::endl;
            res.status = 400;  // 400 Bad Request
            break;
        case ParseError::RequestDoesNotFollowSchema:
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

Expected<int, ParseError> ParseInt(const std::string& value, int min, int max)
{
    // TODO: constexpr initialization doesn't work lol
    static const Expected<int, ParseError> err(ParseError::None);  // TODO: better error

    try
    {
        int val = std::stoi(value);
        if (val < min || val > max)
            return err;

        return Expected<int, ParseError>(std::move(val));  // TODO: ugly
    }
    catch (const std::exception& ex)
    {
        return err;
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

    auto quitHandler = [&server, &isRunning](const Req& req, Res& res)
    {
        std::cout << "Server got shutdown signal, shutting down threads..." << std::endl;
        server.stop();
        isRunning.store(false);
    };
    server.Post("/quit", quitHandler);

    auto startHandler = [&state, &isLeapDriverActive](const Req& req, Res& res)
    {
        auto result = ParseRequest<Start>(req.body);
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
        auto result = ParseRequest<EventClick>(req.body);
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
        auto result = ParseRequest<EventKeystroke>(req.body);
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
        auto result = ParseRequest<EventFieldCompletion>(req.body);
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
        auto result = ParseRequest<EventTaskCompletion>(req.body);
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

    std::cout << "Shutting down HTTP thread..." << std::endl;
}

template <RequestData T>
Expected<T, ParseError> ParseRequest(std::string jsonRequest)
{
    std::string schemaString = GetRequestSchema<T>();
    rapidjson::Document rawSchemaDocument;
    rawSchemaDocument.Parse(schemaString);
    if (rawSchemaDocument.HasParseError())
        return Expected<T, ParseError>(ParseError::SchemaNotValidJSON);

    rapidjson::Document requestDocument;
    requestDocument.Parse(jsonRequest);
    if (requestDocument.HasParseError())
        return Expected<T, ParseError>(ParseError::RequestNotValidJSON);

    rapidjson::SchemaDocument schema(rawSchemaDocument);
    rapidjson::SchemaValidator validator(schema);
    if (!requestDocument.Accept(validator))
        return Expected<T, ParseError>(ParseError::RequestDoesNotFollowSchema);

    // TODO: maybe don't require an rvalue reference in the constructor
    return Expected<T, ParseError>(DeserializeRequest<T>(requestDocument));
}

template <>
Start DeserializeRequest<Start>(rapidjson::Document& requestDocument)
{
    Start request;
    request.userId = requestDocument["userId"].GetInt();
    return request;
}

template <>
EventFieldCompletion DeserializeRequest<EventFieldCompletion>(rapidjson::Document& requestDocument)
{
    EventFieldCompletion request;
    request.data.timestampMillis = requestDocument["timestampMillis"].GetUint64();
    request.data.fieldIndex = requestDocument["fieldIndex"].GetInt();
    return request;
}

template <>
EventTaskCompletion DeserializeRequest<EventTaskCompletion>(rapidjson::Document& requestDocument)
{
    EventTaskCompletion request;
    request.data.timestampMillis = requestDocument["timestampMillis"].GetUint64();
    request.data.taskIndex = requestDocument["taskIndex"].GetInt();
    return request;
}

template <>
EventClick DeserializeRequest<EventClick>(rapidjson::Document& requestDocument)
{
    EventClick request;
    for (auto it = requestDocument.Begin(); it != requestDocument.End(); ++it)
    {
        std::string locationString = (*it)["location"].GetString();
        Logging::Events::ClickLocation location;
        if (locationString == "OutOfBounds")
            location = Logging::Events::ClickLocation::OutOfBounds;
        else if (locationString == "Background")
            location = Logging::Events::ClickLocation::Background;
        else if (locationString == "TextField")
            location = Logging::Events::ClickLocation::TextField;
        else if (locationString == "Button")
            location = Logging::Events::ClickLocation::Button;
        else
            location = Logging::Events::ClickLocation::OutOfBounds;

        Logging::Events::Click event;
        event.timestampMillis = (*it)["timestampMillis"].GetUint64();
        event.wasCorrect = (*it)["wasCorrect"].GetBool();
        event.location = location;
        request.data.push_back(event);
    }
    return request;
}

template <>
EventKeystroke DeserializeRequest<EventKeystroke>(rapidjson::Document& requestDocument)
{
    EventKeystroke request;
    for (auto it = requestDocument.Begin(); it != requestDocument.End(); ++it)
    {
        Logging::Events::Keystroke event;
        event.timestampMillis = (*it)["timestampMillis"].GetUint64();
        event.key = (*it)["key"].GetString();
        event.wasCorrect = (*it)["wasCorrect"].GetBool();
        request.data.push_back(event);
    }
    return request;
}

template <>
std::string GetRequestSchema<Start>()
{
    return R"(
{
    "title": "Request_Start",
    "type": "object",
    "properties": {
        "userId": {
            "type": "integer",
            "description": "The user ID to initialize a user study with."
        }
    },
    "required": ["userId"],
    "unevaluatedProperties": false
}
    )";
}

template <>
std::string GetRequestSchema<EventFieldCompletion>()
{
    return R"(
{
    "title": "Request_Events_Field",
    "type": "object",
    "properties": {
        "timestampMillis": {
            "type": "integer",
            "description": "The Unix time (in milliseconds) at which the event occurred."
        },
        "fieldIndex": {
            "type": "integer",
            "description": "The index of the field that was completed."
        }
    },
    "requried": ["timestampMillis", "fieldIndex"],
    "unevaluatedProperties": false
}
    )";
}

template <>
std::string GetRequestSchema<EventTaskCompletion>()
{
    return R"(
{
    "title": "Request_Events_Task",
    "type": "object",
    "properties": {
        "timestampMillis": {
            "type": "integer",
            "description": "The Unix time (in milliseconds) at which the event occurred."
        },
        "taskIndex": {
            "type": "integer",
            "description": "The index of the task that was completed."
        }
    },
    "requried": ["timestampMillis", "taskIndex"],
    "unevaluatedProperties": false
}
    )";
}

template <>
std::string GetRequestSchema<EventClick>()
{
    return R"(
{
    "title": "Request_Events_Click",
    "type": "array",
    "items": {
        "$ref": "#/$defs/click"
    },
    "$defs": {
        "click": {
            "type": "object",
            "properties": {
                "timestampMillis": {
                    "type": "integer",
                    "description": "The Unix time (in milliseconds) at which the event occurred."
                },
                "location": {
                    "type": "string",
                    "description": "The location on the page that the click was heard at."
                },
                "wasCorrect": {
                    "type": "boolean",
                    "description": "Did the click hit the part of the page it was supposed to at this time?"
                }
            },
            "required": ["timestampMillis", "location", "wasCorrect"],
            "unevaluatedProperties": false
        }
    }
}
    )";
}

template <>
std::string GetRequestSchema<EventKeystroke>()
{
    return R"(
{
    "title": "Request_Events_Keystroke",
    "type": "array",
    "items": {
        "$ref": "#/$defs/keystroke"
    },
    "$defs": {
        "keystroke": {
            "type": "object",
            "properties": {
                "timestampMillis": {
                    "type": "integer",
                    "description": "The Unix time (in milliseconds) at which the event occurred."
                },
                "keycode": {
                    "type": "string",
                    "description": "The key that was pressed."
                },
                "wasCorrect": {
                    "type": "boolean",
                    "description": "Was this character valid input?"
                }
            },
            "required": ["timestampMillis", "keycode", "wasCorrect"],
            "unevaluatedProperties": false
        }
    }
}
    )";
}

}  // namespace Http