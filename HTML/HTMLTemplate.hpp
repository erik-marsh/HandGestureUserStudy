#pragma once

#include <string>
#include <vector>

namespace HTML
{

class HTMLTemplate
{
   public:
    HTMLTemplate(const std::string& filename, const std::string& marker = "{% expected_text %}");

    void Substitute(std::vector<std::string>& strs);
    std::string GetSubstitution() const;
    std::string GetTemplate() const;

    const int GetNumSubstitutions() const;

   private:
    std::vector<std::string> chunks;
    std::vector<std::string> substitutions;

    int numSubstitutions;
    const std::string markerPattern;
};

}  // namespace HTML