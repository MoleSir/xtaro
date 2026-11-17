#pragma once

#include <string_view>
namespace xtaro::parse 
{

    enum class JsonType 
    {
        Null    = 0,
        Boolean = 1, 
        Integer = 2,
        Decimal = 3,
        String  = 4,
        Array   = 5,
        Object  = 6,
    };

    auto jsonTypeToString(JsonType type) -> std::string_view;

}