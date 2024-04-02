#pragma once

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/schema.h>

#include <Helpers/Expected.hpp>
#include <Programs/UserStudy/Logging.hpp>
#include <string_view>
#include <vector>

namespace Helpers
{

// https://stackoverflow.com/questions/15911890/overriding-return-type-in-function-template-specialization
// note that the specializations on the struct types are not necessary
// was this a change made in a later c++ standard?
struct RequestData_t
{
};

enum class ParseError
{
    None = 0,
    SchemaNotValidJSON,
    RequestNotValidJSON,
    RequestDoesNotFollowSchema
};

// I don't exactly understand the rationale,
// but std::derived_from requires that the derived class
// inherit from the base class publically.
template <typename T>
concept RequestData = std::derived_from<T, RequestData_t>;

// Function declarations
template <RequestData T>
Expected<T, ParseError> ParseRequest(std::string jsonRequest);

// struct definitions
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

}  // namespace Helpers