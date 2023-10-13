#include "UserIDLock.hpp"

#include <fstream>
#include <iostream>
#include <format>

namespace Helpers
{

UserIDLock::UserIDLock(const std::string& lockFilename) : filename(lockFilename)
{
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line))
    {
        int id = std::stoi(line);
        lockedIds.push_back(id);
        std::cout << std::format("[User IDs] {} is unavailable.\n", id);
    }
}

UserIDLock::~UserIDLock()
{
    std::ofstream file(filename);
    for (auto it = lockedIds.begin(); it != lockedIds.end(); ++it)
        file << *it << "\n";
}

void UserIDLock::Lock(int id)
{
    if (IsLocked(id))
        return;

    lockedIds.push_back(id);
}

bool UserIDLock::IsLocked(int id)
{
    for (auto it = lockedIds.begin(); it != lockedIds.end(); ++it)
        if (id == *it)
            return true;
    
    return false;
}

}  // namespace Helpers