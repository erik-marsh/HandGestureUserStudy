#pragma once

#include <httplib.h>

#include <Helpers/Expected.hpp>
#include <Helpers/JSONEvents.hpp>
#include <exception>
#include <format>
#include <sstream>

namespace Helpers
{

namespace
{

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

auto postRoutingDebugPrint = [](const httplib::Request& req, httplib::Response& res)
{
    if (req.method == "POST")
        std::cout << std::format("[HTTP] {} {} => {}\n", req.method, req.path, req.body);
    else
        std::cout << std::format("[HTTP] {} {}\n", req.method, req.path);
};

auto parseErrorHandler =
    [](const httplib::Request& req, httplib::Response& res, Helpers::ParseError error)
{
    switch (error)
    {
        case Helpers::ParseError::SchemaNotValidJSON:
            std::cout << "[HTTP] Error: Schema was not valid.\n";
            res.status = 500;  // 500 Internal Server Error
            break;
        case Helpers::ParseError::RequestNotValidJSON:
            std::cout << "[HTTP] Error: Request was not valid JSON.\n";
            res.status = 400;  // 400 Bad Request
            break;
        case Helpers::ParseError::RequestDoesNotFollowSchema:
            std::cout << "[HTTP] Error: Request did not adhere to schema.\n";
            res.status = 400;  // 400 Bad Request
            break;
        default:
            std::cout << "[HTTP] Error: Parse failed but there was no error?\n";
            res.status = 500;  // 500 Internal Server Error
            break;
    }
};

auto errorHandler = [](const httplib::Request& req, httplib::Response& res)
{ res.set_content(std::format("<h1>Error {}</h1>", res.status), "text/html"); };

auto exceptionHandler =
    [](const httplib::Request& req, httplib::Response& res, std::exception_ptr exp)
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
};

}  // namespace

}  // namespace Helpers