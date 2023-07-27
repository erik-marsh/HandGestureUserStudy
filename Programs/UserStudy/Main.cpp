#include <LeapC.h>

#include <Input/SimulatedMouse.hpp>
#include <cstring>
#include <iostream>
#include <thread>

#include "HttpServer.hpp"
#include "LeapDriver.hpp"
#include "Logging.hpp"
#include "SyncState.hpp"
#include "Visualizer.hpp"

int PrintHelp(bool isBadUsage);
int RunMouseConfigure();
int RunUserStudy();

int main(int argc, char** argv)
{
    if (argc > 2)
        return PrintHelp(true);

    if (argc == 2)
    {
        if (!std::strcmp(argv[1], "--help") || !std::strcmp(argv[1], "-h"))
            return PrintHelp(false);
        if (!std::strcmp(argv[1], "--configure-mouse") || !std::strcmp(argv[1], "-c"))
            return RunMouseConfigure();
        else
            return PrintHelp(true);
    }

    return RunUserStudy();
}

int PrintHelp(bool isBadUsage)
{
    std::cout
        << "Usage: .\\handGestureUserStudy <params>\n"
        << "Valid parameters:\n"
        << "    <no parameters> -> Run user study as normal\n"
        << "    --configure-mouse, -c -> Moves the mouse to the location that it will be reset "
        << "to during the user study.\n"
        << "                         The monitor that the mouse moves to is the monitor the "
        << "browser window should be on.\n"
        << "    --help, -h -> Shows this message." << std::endl;
    return static_cast<int>(isBadUsage);
}

int RunMouseConfigure()
{
    Input::Mouse::MoveAbsolute(100, 100);
    return 0;
}

int RunUserStudy()
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