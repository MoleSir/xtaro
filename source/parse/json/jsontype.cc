#include "./jsontype.hh"
#include <utility/debug/debug.hh>

namespace xtaro::parse 
{  

    auto jsonTypeToString(JsonType type) -> std::string_view
    {
        switch (type) 
        {
            case JsonType::Null: return "null";
            case JsonType::Boolean: return "boolean";
            case JsonType::Integer: return "integer";
            case JsonType::Decimal: return "decimal";
            case JsonType::String: return "string";
            case JsonType::Array: return "array";
            case JsonType::Object: return "object";
        }
        debug::unreachable();
    }

}