#pragma once

#include <array>
#include <string_view>

// TODO: move this eventually?
namespace Helpers::StringPools
{

using namespace std::literals;

namespace
{

constexpr std::array EmailAddresses = {"john.doe@example.com"sv,    "john.doe@gmail.com"sv,
                                       "john.doe@protonmail.com"sv, "mary.sue@example.com"sv,
                                       "mary.sue@gmail.com"sv,      "mary.sue@protonmail.com"sv};
constexpr std::array CardNumbers = {"1111-2222-3333-4444"sv, "1234-1234-5678-5678"sv};
constexpr std::array DateOfBirth = {"1/1/1970"sv, "12/2/1997"sv, "05/18/2002"sv, "09/27/1991"sv};
constexpr std::array IdNumbers = {"123-45-6789"sv, "00011001"sv, "1234-5555"sv};
constexpr std::array Names = {"John Doe"sv, "Mary Sue"sv, "Sir Joe Example"sv, "Gary Stu"sv};
constexpr std::array PhysicalAddresses = {"100 W. 1st St."sv, "432 W. 1st St. Apt. 103"sv,
                                          "234 E. 1st St. Apt. 307"sv, "234 E. 1st St. Apt. 201"sv};
constexpr std::array EmailBodyText = {
    "I am conducting a research study to assess performance and user satisfaction with regards to using hand gestures as a way to control a computer cursor. I am currently looking for 12 participants to conduct a user study to advance this research. Participants in this study will be asked to perform several short typing tasks with cursor movement in between typing tasks. The different devices for cursor control are a standard computer mouse, a pair of hand-tracking gloves, and a Leap Motion optical hand tracker. The entire process should take about an hour to complete."sv,
    "If you are interested in participating, note that the gloves used in the study are of a specific size. To participate, your hands must be roughly the size of the glove, which itself has a length of 7 inches and a width of 4.5 inches. If your hand measurements are roughly within an inch of tolerance of these two values, and you wish to participate in this user study, please email John Doe at john.doe@gmail.com to set up a time that works best for you to complete the study."sv};

}  // namespace

}  // namespace Helpers::StringPools