#include "HttpServer.hpp"

#include <httplib.h>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

#include <iostream>

namespace Http
{

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
                [&server, &isRunning](const httplib::Request& req, httplib::Response& res)
                {
                    std::cout << "Server got shutdown signal, shutting down threads..." << std::endl;
                    server.stop();
                    isRunning.store(false);
                });

    server.listen("localhost", 5000);

    std::cout << "Shutting down HTTP thread..." << std::endl;
}

}  // namespace Http