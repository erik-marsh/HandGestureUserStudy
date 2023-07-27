#include <LeapC.h>

#include <thread>

#include "HttpServer.hpp"
#include "LeapDriver.hpp"
#include "Logging.hpp"
#include "SyncState.hpp"
#include "Visualizer.hpp"

int main()
{
    Input::Leap::LeapConnection connection;
    while (!connection.IsConnected())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    Renderables renderables{};
    std::mutex renderableCopyMutex;
    std::atomic<bool> isRunning(true);
    std::atomic<bool> isLeapDriverActive(true);

    SyncState syncState(connection, renderables, renderableCopyMutex, isRunning,
                        isLeapDriverActive);
    auto r_syncState = std::ref(syncState);

    std::thread httpThread(Http::HttpServerLoop, r_syncState);
    std::thread driverThread(Input::DriverLoop, r_syncState);
    std::thread renderThread(Visualization::RenderLoop, r_syncState);

    renderThread.join();
    driverThread.join();
    httpThread.join();

    return 0;
}