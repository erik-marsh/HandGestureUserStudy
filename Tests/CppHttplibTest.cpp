#include <iostream>

#include "../cpp-httplib/httplib.h"

int main()
{
    httplib::Server server;

    server.Get("/test", [](const httplib::Request& req, httplib::Response& res)
               { res.set_content("Hello, world!", "text/plain"); });

    server.Post("/test", [](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Got POST request:\n" << req.body << std::endl;
    });

    server.listen("localhost", 5000);
    return 0;
}