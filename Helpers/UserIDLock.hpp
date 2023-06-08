#pragma once

#include <vector>
#include <string>

namespace Helpers
{

/// @brief Manages a list of user IDs that cannot be used by the software anymore.
///        Used to prevent overwriting previous data.
/// @remark Not thread safe in the slightest!
class UserIDLock
{
public:
    UserIDLock(const std::string& lockFilename);
    ~UserIDLock();

    void Lock(int id);
    bool IsLocked(int id);

private:
    const std::string filename;
    std::vector<int> lockedIds;
};

}