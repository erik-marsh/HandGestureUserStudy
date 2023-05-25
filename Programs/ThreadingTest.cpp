#include <atomic>
#include <thread>

#include "Visualizer.hpp"
#include "LeapDriver.hpp"

int main()
{
    Input::Leap::LeapConnection connection;
    while (!connection.IsConnected()) std::this_thread::sleep_for(std::chrono::milliseconds(100));

    Visualization::Renderables renderables;
    renderables.leapDebugString = "Hello, world!";
    renderables.hands = std::vector<LEAP_HAND>();
    renderables.averageFingerDirectionX = 10;
    renderables.averageFingerDirectionY = 10;
    renderables.cursorDirectionX = -10;
    renderables.cursorDirectionY = -10;

    // std::atomic<bool> isRendering(true);
    std::atomic<bool> isRunning(true);

    // std::thread renderThread(Visualization::RenderLoop, std::ref(renderables),
    // std::ref(isRendering));
    std::thread driverThread(Input::DriverLoop, std::ref(connection), std::ref(renderables),
                             std::ref(isRunning));
    std::this_thread::sleep_for(std::chrono::seconds(10));
    // isRendering.store(false);
    // renderThread.join();
    isRunning.store(false);
    driverThread.join();

    return 0;
}