#include <atomic>
#include <thread>

#include "LeapDriver.hpp"
#include "Visualizer.hpp"

// TODO: There are some flaws with this entire approach:
// 1) The leap connection crashes every once in a while when doing multithreading.
// 2) If we update the hand state at 60Hz or less,
//    there is literally no reason to separate the processing and rendering threads.
//    We could probably tick the processing thread once every 1ms,
//    since the processing time seems to be roughly 200-700us every frame.
int main()
{
    Input::Leap::LeapConnection connection;
    while (!connection.IsConnected()) std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // this is cleared inside of Input::DriverLoop
    Visualization::Renderables renderables;

    std::atomic<bool> isRendering(true);
    std::atomic<bool> isRunning(true);

    std::thread renderThread(Visualization::RenderLoop, std::ref(renderables),
                             std::ref(isRendering));
    std::thread driverThread(Input::DriverLoop, std::ref(connection), std::ref(renderables),
                             std::ref(isRunning));
    std::this_thread::sleep_for(std::chrono::seconds(10));
    isRendering.store(false);
    renderThread.join();
    isRunning.store(false);
    driverThread.join();

    return 0;
}