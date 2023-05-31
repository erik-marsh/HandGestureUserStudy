#include <httplib.h>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

#include <iostream>

int main()
{
    httplib::Server server;

    server.Get("/test", [](const httplib::Request& req, httplib::Response& res)
               { res.set_content("Hello, world!", "text/plain"); });

    server.Post("/test",
                [](const httplib::Request& req, httplib::Response& res)
                {
                    std::cout << "Got POST request:\n" << req.body << std::endl;
                    rapidjson::Document d;
                    d.Parse(req.body.c_str());

                    // interestingly, we get an assertion fail if this parse fails
                    // i'll take it, but i'm used to seeing exceptions in these kinds of cases
                    std::cout << d["value"].GetInt() << std::endl;
                });

    server.Get("/stop",
               [&server](const httplib::Request& req, httplib::Response& res) { server.stop(); });

    std::cout << "Launching server" << std::endl;
    // this blocks, and server.stop() needs to be called to properly stop the server
    server.listen("localhost", 5000);
    std::cout << "Exited gracefully" << std::endl;
    return 0;
}