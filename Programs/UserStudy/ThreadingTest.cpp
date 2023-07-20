#include <LeapC.h>

#include <atomic>
#include <mutex>
#include <thread>

#include "HttpServer.hpp"
#include "LeapDriver.hpp"
#include "Logging.hpp"
#include "Visualizer.hpp"

int main()
{
    Input::Leap::LeapConnection connection;
    while (!connection.IsConnected())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    Visualization::Renderables renderables{};
    std::mutex renderableCopyMutex;
    std::atomic<bool> isRunning(true);
    std::atomic<bool> isLeapDriverActive(true);

    auto r_isRunning = std::ref(isRunning);
    auto r_isLeapDriverActive = std::ref(isLeapDriverActive);
    auto r_renderables = std::ref(renderables);
    auto r_renderableCopyMutex = std::ref(renderableCopyMutex);
    auto r_connection = std::ref(connection);

    std::thread httpThread(Http::HttpServerLoop, r_isRunning, r_isLeapDriverActive);
    std::thread driverThread(Input::DriverLoop, r_connection, r_renderables, r_isRunning,
                             r_isLeapDriverActive, r_renderableCopyMutex);
    std::thread renderThread(Visualization::RenderLoop, r_renderables, r_isRunning,
                             r_renderableCopyMutex);

    renderThread.join();
    driverThread.join();
    httpThread.join();

    return 0;
}