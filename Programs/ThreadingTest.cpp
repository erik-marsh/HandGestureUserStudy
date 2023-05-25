#include "Visualizer.hpp"

#include <atomic>
#include <thread>

int main()
{
    Visualization::Renderables renderables;
    renderables.leapDebugString = "Hello, world!";
    renderables.hands = std::vector<LEAP_HAND>();
    renderables.averageFingerDirectionX = 10;
    renderables.averageFingerDirectionY = 10;
    renderables.cursorDirectionX = -10;
    renderables.cursorDirectionY = -10;

    std::atomic<bool> isRendering(true);

    std::thread renderThread(Visualization::RenderLoop, std::ref(renderables), std::ref(isRendering));
    std::this_thread::sleep_for(std::chrono::seconds(10));
    isRendering.store(false);
    renderThread.join();
    
    return 0;
}