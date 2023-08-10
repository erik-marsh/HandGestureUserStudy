#pragma once

#include <array>
#include <random>
#include <string>
#include <string_view>

// TODO: move this eventually?
namespace Helpers::StringPools
{

namespace
{

template <size_t size>
std::string SelectRandom(const std::array<std::string_view, size>& array)
{
    static std::random_device device{};
    static std::mt19937 generator{device()};
    static std::uniform_int_distribution<int> distribution(0, array.size() - 1);
    int index = distribution(generator);
    return std::string(array[index]);
}

using namespace std::literals;

constexpr std::array EmailAddresses = {"owl@gmail.com"sv,         "finch@gmail.com"sv,
                                       "quail@gmail.com"sv,       "pidgeon@gmail.com"sv,
                                       "admin@bigcorp.org"sv,     "founder@bigcorp.org"sv,
                                       "scheduler@bigcorp.org"sv, "prototypes.lead@tech.com"sv,
                                       "payroll@tech.com"sv,      "manufacture@tech.com"sv};

constexpr std::array CardNumbers = {"1111-2222-3333-4444"sv,
                                    "9876-9876-1111-2222"sv,
                                    "1234-5678-9999-1111"sv,
                                    "1234-1234-5678-5678"sv,
                                    "98765432"sv,
                                    "12345432"sv,
                                    "12345678"sv};

constexpr std::array DateOfBirth = {"1/1/1970"sv,   "12/2/1997"sv, "05/18/2002"sv,
                                    "09/27/1991"sv, "4/16/1954"sv, "9/1/2004"sv};

constexpr std::array IdNumbers = {"123-45-6789"sv, "00011001"sv, "1234-5555"sv, "123-45-678"sv,
                                  "123-12-9876"sv, "02022002"sv, "1234-4321"sv, "523-45-118"sv};

constexpr std::array Names = {"Charles Mingus"sv, "Paul Desmond"sv,  "Miles Davis"sv,
                              "Roy Hargrove"sv,   "Johnny Cash"sv,   "Denzel Curry"sv,
                              "Marty Friedman"sv, "Shutoku Mukai"sv, "Hisako Tabuchi"sv,
                              "Kentaro Nakao"sv,  "Ahito Inazawa"sv, "Etsuko Yakushimaru"sv};

constexpr std::array PhysicalAddresses = {"100 W. 1st St."sv, "432 W. 1st St. Apt. 103"sv,
                                          "234 E. 1st St. Apt. 307"sv, "234 E. 1st St. Apt. 201"sv};
                                          
constexpr std::array EmailBodyText = {
    "Received, thanks."sv,
    "I'll handle that after lunch. Thanks."sv,
    "No worries."sv,
    "We will need it done by the end of the quarter."sv,
    "I am conducting a research study to assess performance and user satisfaction with regards to using hand gestures as a way to control a computer cursor."sv,
    "The entire process should take no more than an hour to complete."sv};

}  // namespace

}  // namespace Helpers::StringPools