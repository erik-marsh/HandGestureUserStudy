#include "HTMLTemplate.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

namespace HTML
{

HTMLTemplate::HTMLTemplate(const std::string& filename, const std::string& marker)
    : markerPattern(marker)
{
    std::ifstream file(filename);

    std::string chunk;
    std::string line;
    while (std::getline(file, line))
    {
        size_t markerLoc = line.find(marker);
        if (markerLoc == std::string::npos)
        {
            chunk.append(line);
            chunk.append("\n");  // since std::getline eats the newline
            continue;
        }

        size_t lastOrigin = 0;
        do
        {
            chunk.append(line, lastOrigin, markerLoc - lastOrigin);
            chunks.push_back(chunk);
            chunk = "";

            lastOrigin = markerLoc + marker.size();
            markerLoc = line.find(marker, lastOrigin);
        } while (markerLoc != std::string::npos);

        chunk.append(line, lastOrigin, line.size() - lastOrigin);
        chunk.append("\n");  // since std::getline eats the newline
    }

    chunks.push_back(chunk);
    numSubstitutions = chunks.size() - 1;
}

void HTMLTemplate::Substitute(const std::vector<std::string>& strs)
{
    substitutions.clear();
    for (auto it = strs.begin(); it != strs.end(); ++it)
        substitutions.push_back(*it);
}

std::string HTMLTemplate::GetSubstitution() const
{
    std::stringstream ss;
    if (substitutions.size() != numSubstitutions)
    {
        ss << "An HTML document with " << numSubstitutions << " substitution markers must be given "
           << numSubstitutions << " strings to substitute with. (Got " << substitutions.size()
           << " strings).";
        throw std::runtime_error(ss.str());
    }

    for (int i = 0; i < numSubstitutions; i++)
        ss << chunks[i] << substitutions[i];
    ss << *(chunks.end() - 1);
    return ss.str();
}

std::string HTMLTemplate::GetTemplate() const
{
    std::stringstream ss;
    for (int i = 0; i < numSubstitutions; i++)
        ss << chunks[i] << markerPattern;
    ss << *(chunks.end() - 1);
    return ss.str();
}

const int HTMLTemplate::GetNumSubstitutions() const { return numSubstitutions; }

}  // namespace HTML