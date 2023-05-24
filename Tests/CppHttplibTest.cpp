#include <iostream>

#include "../cpp-httplib/httplib.h"
#include "../rapidjson/include/rapidjson/rapidjson.h"
#include "../rapidjson/include/rapidjson/document.h"

int main()
{
    httplib::Server server;

    server.Get("/test", [](const httplib::Request& req, httplib::Response& res)
               { res.set_content("Hello, world!", "text/plain"); });

    server.Post("/test", [](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Got POST request:\n" << req.body << std::endl;
        rapidjson::Document d;
        d.Parse(req.body.c_str());

        // interestingly, we get an assertion fail if this parse fails
        // i'll take it, but i'm used to seeing exceptions in these kinds of cases
        std::cout << d["value"].GetInt() << std::endl;
    });

    server.listen("localhost", 5000);
    return 0;
}