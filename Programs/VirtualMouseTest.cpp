#include <Input/SimulatedMouse.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

const std::string USAGE_MESSAGE =
    "./virtualMouse <absolute | relative> <vert> <horiz> <click? true | false>";

int ExitFailure()
{
    printf(USAGE_MESSAGE.c_str());
    return -1;
}

// ./<exename> <absolute | relative> <vert> <horiz> <click?>
int main(int argc, char** argv)
{
    if (argc != 5) return ExitFailure();

    bool useAbsolute;
    if (!std::strcmp(argv[1], "absolute"))
        useAbsolute = true;
    else if (!std::strcmp(argv[1], "relative"))
        useAbsolute = false;
    else
        return ExitFailure();

    int verticalMotion = atoi(argv[2]);
    int horizontalMotion = atoi(argv[3]);

    bool leftClick;
    if (!std::strcmp(argv[4], "true"))
        leftClick = true;
    else if (!std::strcmp(argv[4], "false"))
        leftClick = false;
    else
        return ExitFailure();

    auto startPoint = Input::Mouse::QueryMousePosition();
    printf("Mouse position before move: (x=%d, y=%d)\n", startPoint.first, startPoint.second);

    if (useAbsolute)
        Input::Mouse::MoveAbsolute(horizontalMotion, verticalMotion);
    else
        Input::Mouse::MoveRelative(horizontalMotion, verticalMotion);

    if (leftClick) Input::Mouse::LeftClick();

    auto endPoint = Input::Mouse::QueryMousePosition();
    printf("Mouse position after move: (x=%d, y=%d)\n", endPoint.first, endPoint.second);
    printf("Mouse position differential: (x=%d, y=%d)\n", startPoint.first - endPoint.first,
           startPoint.second - endPoint.second);

    return 0;
}