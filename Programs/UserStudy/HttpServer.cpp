#include "HttpServer.hpp"

#include <httplib.h>
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/schema.h>

#include <Helpers/Expected.hpp>
#include <Helpers/IsAnyOf.hpp>
#include <iostream>

namespace Http
{

using Req = httplib::Request;
using Res = httplib::Response;

std::string SCHEMA_START = R"(
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

void HttpServerLoop(std::atomic<bool>& isRunning)
{
    std::cout << "Starting HTTP thread..." << std::endl;
    httplib::Server server;

    if (!server.set_mount_point("/", "./www/"))
    {
        std::cout << "Unable to set mount point" << std::endl;
        isRunning.store(false);
        return;
    }

    server.Post("/quit",
                [&server, &isRunning](const Req& req, Res& res)
                {
                    std::cout << "Server got shutdown signal, shutting down threads..."
                              << std::endl;
                    server.stop();
                    isRunning.store(false);
                });

    server.Post("/start",
                [](const Req& req, Res& res)
                {
                    auto result = ParseRequest<Start>(req.body);
                    if (result.HasValue())
                    {
                        std::cout << "Starting user study for user id " << result.Value().userId << std::endl;
                        res.status = 200;  // 200 OK
                        return;
                    }

                    switch (result.Error())
                    {
                        case ParseError::SchemaNotValidJSON:
                            std::cout << "Schema for Start was not valid." << std::endl;
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
                });

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
std::string GetRequestSchema<Start>()
{
    return SCHEMA_START;
}

}  // namespace Http