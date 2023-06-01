#include "../UserStudy/Logging.hpp"

#include <thread>
#include <iostream>

int main()
{
    for (int i = 0; i < 300; i++)
        Logging::Log(Logging::Events::CursorPosition{});
    
    std::cout << "File should be umodified. Sleeping for 10s..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    for (int i = 0; i < 1000; i++)
        Logging::Log(Logging::Events::Keystroke{});

    std::cout << "File should contain 1000 lines. Sleeping for 10s..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "File should contain 1300 lines. Done." << std::endl;
    return 0;
}