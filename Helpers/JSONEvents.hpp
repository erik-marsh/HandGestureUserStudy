#pragma once

#include <rapidjson/document.h>

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

// forward template declarations
template Expected<Start, ParseError> ParseRequest(std::string);
template Expected<EventFieldCompletion, ParseError> ParseRequest(std::string);
template Expected<EventTaskCompletion, ParseError> ParseRequest(std::string);
template Expected<EventClick, ParseError> ParseRequest(std::string);
template Expected<EventKeystroke, ParseError> ParseRequest(std::string);

}  // namespace Helpers