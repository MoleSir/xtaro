#include "string.hh"

#include <exception>
#include <format>
#include <ranges>
#include <stdexcept>
#include <utility/debug/debug.hh>

#include <string>
#include <cstdlib>
#include <functional>
#include <charconv>
#include <algorithm>

namespace xtaro::util
{

    auto isWhiteChar(char ch) -> bool {
        return ch == ' ' || ch == '\r' || ch == '\n' || ch == '\a' || ch == '\t';
    }

    char isDigit(const char ch)
    {
        return ch >= '0' && ch <= '9';
    }

    auto split(std::string_view delims, std::string_view input) -> std::vector<std::string_view>
    {
        std::vector<std::string_view> result {};
        std::size_t begin {0};
        std::size_t end {0};

        while ((end = input.find(delims.data(), begin)) != std::string::npos)
        {
            result.emplace_back(input.substr(begin, end - begin));
            begin = end + delims.size();
        }

        if (begin != input.size())
            result.emplace_back(input.substr(begin, input.size() - begin));

        return result;
    }

    auto split(char delim, std::string_view input) -> std::vector<std::string_view>
    {
        std::vector<std::string_view> result {};
        std::size_t begin {0};
        std::size_t end {0};

        while ((end = input.find(delim, begin)) != std::string::npos)
        {
            result.emplace_back(input.substr(begin, end - begin));
            begin = end + 1;
        }

        if (begin != input.size())
            result.emplace_back(input.substr(begin, input.size() - begin));
            
        return result;
    }

    static auto doSplitAll(std::function<bool(char)> isDelims, std::string_view input) -> std::vector<std::string_view>
    {
        std::vector<std::string_view> result {};
        std::size_t begin {0};
        std::size_t end {0};

        while (begin < input.size())
        {
            if (isDelims(input[begin])) 
            {
                begin += 1;
                continue;
            }

            // input[begin] is not 'delims', find next 'delims'
            end = begin + 1;
            while (true)
            {
                if (end >= input.size() || isDelims(input[end]))
                    break;
                end += 1;
            }
            // 'input[begin : end)' is a token
            result.emplace_back(input.substr(begin, end - begin));
            // Update begin
            begin = end + 1;
        }

        return result;
    }

    auto splitAll(char delim, std::string_view input) -> std::vector<std::string_view>
    {
        auto isDelims = [delim] (char ch) {
            return ch == delim;
        };

        return doSplitAll(isDelims, input);
    }

    auto splitAll(std::string_view delims, std::string_view input) -> std::vector<std::string_view>
    {
        auto isDelims = [delims] (char ch) {
            return delims.find(ch) != std::string::npos;
        };

        return doSplitAll(isDelims, input);
    }

    auto stringToDouble(std::string_view const str) -> double
    {
        try 
        {    
            return std::stod(std::string(str));
        }
        catch (const std::exception& err)
        {
            throw std::runtime_error(std::format("Can't parse '{}' to double for '{}'", str, err.what())); 
        }
        return 0.0; // unreach 
    }

    auto stringToInteger(std::string_view const str) -> int
    {
        int value;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);

        if (ec != std::errc())
            throw std::runtime_error(std::format("Can't parse '{}' to integer", str));

        return value;
    }
    
    auto stringTrim(std::string_view s) -> std::string_view {
        s.remove_prefix(std::find_if(s.begin(), s.end(), [](char ch){ return !isWhiteChar(ch); }) - s.begin());
        s.remove_suffix(std::find_if(s.rbegin(), s.rend(), [](char ch){ return !isWhiteChar(ch); }) - s.rbegin());
        return s;
    }

    auto stringToLowercase(std::string_view s) -> std::string
    {
        auto view =  s | std::views::transform([](char c) -> char {
            if (c >= 'A' && c <= 'Z') {
                return c + ('a' - 'A');
            }
            return c;
        });
        return std::string{view.begin(), view.end()};
    }

}