#include "JSONEvents.hpp"

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/schema.h>

namespace Helpers
{

template <RequestData T>
struct RequestDataReturn_t
{
    using type = T;
};

template <RequestData T>
typename RequestDataReturn_t<T>::type DeserializeRequest(rapidjson::Document& requestDocument);

template <RequestData T>
consteval std::string_view GetRequestSchema();

template <RequestData T>
Expected<T, ParseError> ParseRequest(std::string jsonRequest)
{
    constexpr std::string_view schemaString = GetRequestSchema<T>();
    rapidjson::Document rawSchemaDocument;
    rawSchemaDocument.Parse(schemaString.data(), schemaString.size());
    if (rawSchemaDocument.HasParseError())
        return Expected<T, ParseError>(ParseError::SchemaNotValidJSON);

    rapidjson::Document requestDocument;
    requestDocument.Parse(jsonRequest.data(), jsonRequest.size());
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
consteval std::string_view GetRequestSchema<Start>()
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
consteval std::string_view GetRequestSchema<EventFieldCompletion>()
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
consteval std::string_view GetRequestSchema<EventTaskCompletion>()
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
consteval std::string_view GetRequestSchema<EventClick>()
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
consteval std::string_view GetRequestSchema<EventKeystroke>()
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

}