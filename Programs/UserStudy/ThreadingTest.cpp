#include <atomic>
#include <thread>

#include "HttpServer.hpp"
#include "LeapDriver.hpp"
#include "Logging.hpp"
#include "Visualizer.hpp"

// Note: if we update the hand state at 60Hz or less,
// there is literally no reason to separate the processing and rendering threads.
// We could probably tick the processing thread once every 1ms,
// since the processing time seems to be roughly 200-700us every frame.
int main()
{
    // initialize our resources
    Input::Leap::LeapConnection connection;
    while (!connection.IsConnected())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // this is cleared inside of Input::DriverLoop
    Visualization::Renderables renderables;
    std::atomic<bool> isRunning(true);
    std::atomic<bool> isLeapDriverActive(true);

    auto r_isRunning = std::ref(isRunning);
    auto r_isLeapDriverActive = std::ref(isLeapDriverActive);
    auto r_renderables = std::ref(renderables);
    auto r_connection = std::ref(connection);

    std::thread httpThread(Http::HttpServerLoop, r_isRunning, r_isLeapDriverActive);
    std::thread driverThread(Input::DriverLoop, r_connection, r_renderables, r_isRunning,
                             r_isLeapDriverActive);
    std::thread renderThread(Visualization::RenderLoop, r_renderables, r_isRunning);

    renderThread.join();
    driverThread.join();
    httpThread.join();

    return 0;
}