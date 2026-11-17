#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cstdarg>

namespace xtaro::util
{
    char isDigit(const char ch);

    auto split(char delim, std::string_view input) -> std::vector<std::string_view>;
    auto split(std::string_view delims, std::string_view input) -> std::vector<std::string_view>;

    auto splitAll(char delim, std::string_view input) -> std::vector<std::string_view>;
    auto splitAll(std::string_view delims, std::string_view input) -> std::vector<std::string_view>;

    auto stringToDouble(std::string_view const str) -> double;
    auto stringToInteger(std::string_view const str) -> int;

    auto stringTrim(std::string_view s) -> std::string_view;

    auto stringToLowercase(std::string_view s) -> std::string;
    
}