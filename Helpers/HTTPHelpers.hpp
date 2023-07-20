#pragma once

#include <httplib.h>

#include <Helpers/Expected.hpp>
#include <Helpers/JSONEvents.hpp>
#include <sstream>
#include <exception>

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

auto printRequest = [](const httplib::Request& req, httplib::Response& res)
{
    std::stringstream ss;
    ss << "[" << req.path << "] " << req.body << "\n";
    std::cout << ss.str();
};

auto parseErrorHandler =
    [](const httplib::Request& req, httplib::Response& res, Helpers::ParseError error)
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

auto errorHandler = [](const httplib::Request& req, httplib::Response& res)
{
    std::stringstream ss;
    ss << "<h1>Error " << res.status << "</h1>";
    res.set_content(ss.str(), "text/html");
};

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