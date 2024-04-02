#include "JSONEvents.hpp"

namespace Helpers
{

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

}