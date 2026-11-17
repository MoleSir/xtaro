#include "./json.hh"
#include "./jsonscanner.hh"
#include "./jsonparser.hh"
#include "./jsonerror.hh"

#include <cstddef>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <utility/misc/string.hh>
#include <utility/misc/file.hh>
#include <utility/debug/debug.hh>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <sstream>

namespace xtaro::parse
{

    static constexpr auto jsonTypeToInteger(JsonType type) -> std::size_t 
    {
        return static_cast<std::size_t>(type);
    }

    static constexpr auto integerToJsonType(std::size_t i) -> JsonType
    {
        return static_cast<JsonType>(i);
    }

    Json::Json() :
        _value{Null{}} 
    {
    }

    auto Json::null() -> Json
    {
        return Json{};
    }

    auto Json::boolean(bool value) -> Json
    {
        auto j = Json{};
        j._value = value;
        return j;
    }

    auto Json::integer(int value) -> Json
    {
        auto j = Json{};
        j._value = value;
        return j;
    }

    auto Json::decimal(double value) -> Json
    {
        auto j = Json{};
        j._value = value;
        return j;
    }

    auto Json::string(std::string value) -> Json
    {
        auto j = Json{};
        j._value = value;
        return j;
    }

    auto Json::array() -> Json
    {
        auto j = Json{};
        j._value = std::vector<Json>{};
        return j;
    }

    auto Json::object() -> Json
    {
        auto j = Json{};
        j._value = std::unordered_map<std::string, Json>{};
        return j;
    }

    bool Json::isNull() const noexcept {
        return this->checkType(JsonType::Null);
    }

    bool Json::isBoolean() const noexcept {
        return this->checkType(JsonType::Boolean);
    }

    bool Json::isInteger() const noexcept {
        return this->checkType(JsonType::Integer);
    }

    bool Json::isDecimal() const noexcept {
        return this->checkType(JsonType::Decimal);
    }

    bool Json::isString() const noexcept {
        return this->checkType(JsonType::String);
    }

    bool Json::isArray() const noexcept {
        return this->checkType(JsonType::Array);
    }

    bool Json::isObject() const noexcept {
        return this->checkType(JsonType::Object);
    }

    ///////////////////////////////////////////////////////////////////////////

    auto Json::asBoolean() const -> bool 
    {   
        this->ensureTyep(JsonType::Boolean);
        return std::get<jsonTypeToInteger(JsonType::Boolean)>(this->_value);
    }

    auto Json::asInteger() const -> int
    {   
        this->ensureTyep(JsonType::Integer);
        return std::get<jsonTypeToInteger(JsonType::Integer)>(this->_value);
    }

    auto Json::asDecimal() const -> double
    {   
        this->ensureTyep(JsonType::Decimal);
        return std::get<jsonTypeToInteger(JsonType::Decimal)>(this->_value);
    }

    auto Json::asString() const -> std::string_view
    {   
        this->ensureTyep(JsonType::String);
        return std::get<jsonTypeToInteger(JsonType::String)>(this->_value);
    }

    auto Json::asArray() const -> std::span<const Json>
    {   
        this->ensureTyep(JsonType::Array);
        return std::get<jsonTypeToInteger(JsonType::Array)>(this->_value);
    }

    auto Json::asObject() const -> const std::unordered_map<std::string, Json>&
    {   
        this->ensureTyep(JsonType::Object);
        return std::get<jsonTypeToInteger(JsonType::Object)>(this->_value);
    }

    ///////////////////////////////////////////////////////////////////////////

    auto Json::asBoolean() -> bool& 
    {   
        this->ensureTyep(JsonType::Boolean);
        return std::get<static_cast<std::size_t>(JsonType::Boolean)>(this->_value);
    }

    auto Json::asInteger() -> int&
    {   
        this->ensureTyep(JsonType::Integer);
        return std::get<static_cast<std::size_t>(JsonType::Integer)>(this->_value);
    }

    auto Json::asDecimal() -> double&
    {   
        this->ensureTyep(JsonType::Decimal);
        return std::get<static_cast<std::size_t>(JsonType::Decimal)>(this->_value);
    }

    auto Json::asString() -> std::string&
    {   
        this->ensureTyep(JsonType::String);
        return std::get<static_cast<std::size_t>(JsonType::String)>(this->_value);
    }

    auto Json::asArray() -> std::vector<Json>&
    {   
        this->ensureTyep(JsonType::Array);
        return std::get<static_cast<std::size_t>(JsonType::Array)>(this->_value);
    }

    auto Json::asObject() -> std::unordered_map<std::string, Json>&
    {   
        this->ensureTyep(JsonType::Object);
        return std::get<static_cast<std::size_t>(JsonType::Object)>(this->_value);
    }

    ///////////////////////////////////////////////////////////////////////////

    auto Json::at(const std::string& key) const -> const Json&
    {
        auto& map = this->asObject();
        auto res = map.find(key);
        if (res == map.end()) 
            throw JsonKeyError{key};
        return res->second;
    }

    auto Json::at(const std::string& key) -> Json&
    {
        auto& map = this->asObject();
        auto res = map.emplace(key, Json{});
        return res.first->second;
    }

    auto Json::operator [] (const std::string& key) const  -> const Json&
    {
        return this->at(key);
    }

    auto Json::operator [] (const std::string& key) -> const Json&
    {
        return this->at(key);
    }
    
    auto Json::get(const std::string& key) const -> std::optional<const Json*> 
    {
        auto& map = this->asObject();
        auto res = map.find(key);
        if (res == map.end()) 
            return std::nullopt;
        return std::optional<const Json*> {&(res->second)};
    }

    auto Json::get(const std::string& key) -> std::optional<Json*>
    {
        auto& map = this->asObject();
        auto res = map.find(key);
        if (res == map.end()) 
            return std::nullopt;
        return std::optional<Json*>{&(res->second)};
    }

    auto Json::contains(const std::string& key) const -> bool 
    {
        auto& map = this->asObject();
        return map.contains(key);
    }

    auto Json::size() const -> std::size_t
    {
        return this->asObject().size();
    }

    auto Json::insert(std::string key, Json value) -> void
    {
        auto& map = this->asObject();
        map.emplace(std::move(key), std::move(value));
    }

    /////////////////////////////////////////////////////////////////////////////

    auto Json::at(std::size_t index) const -> const Json&
    {
        auto arr = this->asArray();
        if (index >= arr.size()) 
            throw JsonIndexError{index, arr.size()};
        return arr[index]; 
    }

    auto Json::at(std::size_t index) -> Json&
    {
        auto& arr = this->asArray();
        if (index >= arr.size()) 
            throw JsonIndexError{arr.size(), index};
        return arr[index]; 
    }

    auto Json::operator [] (std::size_t index) const -> const Json&
    {
        return this->at(index);
    }
    
    auto Json::operator [] (std::size_t index) -> Json&
    {
        return this->at(index);
    }

    auto Json::get(std::size_t index) const -> std::optional<const Json*> 
    {
        auto arr = this->asArray();
        if (index >= arr.size()) 
            return std::nullopt;
        return std::optional<const Json*>{&arr[index]}; 
    }

    auto Json::get(std::size_t index) -> std::optional<Json*>
    {
        auto& arr = this->asArray();
        if (index >= arr.size()) 
            return std::nullopt;
        return std::optional<Json*>{&arr[index]}; 
    }

    /////////////////////////////////////////////////////////

    auto Json::len() const -> std::size_t
    {
        return this->asArray().size();
    }

    auto Json::push(Json json) -> void
    {
        return this->asArray().push_back(std::move(json));
    }

    /////////////////////////////////////////////////////////////////

    auto Json::toString() -> std::string 
    {
        return this->doToString();
    }

    /////////////////////////////////////////////////////////////////

    void Json::ensureTyep(JsonType type) const 
    {
        if (!this->checkType(type)) 
            throw JsonTypeError{type, this->type()};
    }

    auto Json::checkType(JsonType type) const -> bool
    {
        return this->typeIndex() == static_cast<std::size_t>(type);
    }

    auto Json::typeIndex() const noexcept -> std::size_t
    {
        return this->_value.index();
    }

    auto Json::type() const noexcept -> JsonType
    {
        return integerToJsonType(this->_value.index());
    }

    Json Json::loadFromFile(const std::filesystem::path& filepath) 
    {
        std::string content {};
        try
        {
            content = util::readFile(filepath);
        } 
        catch (const std::exception& err)
        {
            throw std::runtime_error{std::format("file '{}' read failed", filepath.c_str())};
        }

        auto tokens = JsonScanner{std::move(content)}.scan();
        return JsonParser{std::move(tokens)}.parse();
    }

    std::string Json::doToString(std::size_t indent, bool hasName, bool isObjectValue) const
    {
        std::stringstream ss;

        auto printSpace = [&ss] (std::size_t indent) {
            for (std::size_t i = 0; i < indent; ++i)
                ss << "    ";
        };

        switch (this->type())
        {
        case JsonType::Null:
            ss << "null";
            break;
        case JsonType::Boolean:
            ss << (this->asBoolean() == true ? "true" : "false"); 
            break;
        case JsonType::Integer:
            ss << this->asInteger();
            break;
        case JsonType::Decimal:
            ss << this->asDecimal();
            break;
        case JsonType::String:
            ss << '\"' << this->asString() << '\"';
            break;
        case JsonType::Array: 
        {
            if (!hasName)  printSpace(indent);
            ss << '[' << '\n';

            auto array = this->asArray();
            for (auto iter = array.begin(); iter != array.end(); ++iter)
            {
                if (iter != array.begin())
                    ss << ',' << '\n';
                printSpace(indent + 1);
                ss << iter->doToString(indent + 1, true, false);
            }

            ss << '\n';
            printSpace(indent);
            ss << ']';
            break;
        }
        case JsonType::Object:
        {
            if (!hasName) printSpace(indent);
            ss << '{' << '\n';

            auto& map = this->asObject();
            for (auto iter = map.begin(); iter != map.end(); ++iter)
            {
                if (iter != map.begin())
                    ss << ',' << '\n';
                printSpace(indent + 1);
                ss << '\"' << iter->first << '\"' << " : " 
                   << iter->second.doToString(indent + 1, true, true);
            }

            ss << '\n';
            printSpace(indent);
            ss << '}';
            break;
        }
        default:
            break;
        }

        return ss.str();
    }

}
