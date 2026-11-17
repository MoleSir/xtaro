#pragma once

#include "../serde/mod.hh"
#include "./jsontype.hh"

#include <any>
#include <cstddef>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <map>
#include <filesystem>
#include <variant>

namespace xtaro::parse
{
    
    class Json
    {
    public:
        Json();

    public:
        static auto null() -> Json;
        static auto boolean(bool value) -> Json;
        static auto integer(int value) -> Json;
        static auto decimal(double value) -> Json;
        static auto string(std::string value) -> Json;
        static auto array() -> Json;
        static auto object() -> Json;

        static auto loadFromFile(const std::filesystem::path& filepath) -> Json;

    public:
        bool isNull() const noexcept;
        bool isBoolean() const noexcept;
        bool isInteger() const noexcept;   
        bool isDecimal() const noexcept;
        bool isString() const noexcept;
        bool isArray() const noexcept;
        bool isObject() const noexcept;

    public:
        auto asBoolean() const -> bool;
        auto asInteger() const -> int;   
        auto asDecimal() const -> double;
        auto asString() const -> std::string_view;
        auto asArray() const -> std::span<const Json>;
        auto asObject() const -> const std::unordered_map<std::string, Json>&;

    public:
        auto asBoolean() -> bool&;
        auto asInteger() -> int&;   
        auto asDecimal() -> double&;
        auto asString() -> std::string&;
        auto asArray() -> std::vector<Json>&;
        auto asObject() -> std::unordered_map<std::string, Json>&;

    public:
        auto at(const std::string& key) const -> const Json&;
        auto at(const std::string& key) -> Json&;
        auto operator [] (const std::string& key) const  -> const Json&;
        auto operator [] (const std::string& key) -> const Json&;
        auto get(const std::string& key) const -> std::optional<const Json*>;
        auto get(const std::string& key) -> std::optional<Json*>;

        auto contains(const std::string& key) const -> bool;
        auto size() const -> std::size_t;

        auto insert(std::string key, Json value) -> void;

    public:
        auto at(std::size_t index) const -> const Json&;
        auto at(std::size_t index) -> Json&;
        auto operator [] (std::size_t index) const -> const Json&;
        auto operator [] (std::size_t index) -> Json&;
        auto get(std::size_t index) const -> std::optional<const Json*>;
        auto get(std::size_t index) -> std::optional<Json*>;

        auto len() const -> std::size_t;
        auto push(Json json) -> void;

    public:
        auto toString() -> std::string;
        auto type() const noexcept -> JsonType;

    public:
        template <typename Value>
        auto get_to(Value& v) const -> void {
            xtaro::parse::Deserialize<Json, Value>::from(*this, v);
        }

        template <typename Value>
        auto set_to(const Value& v) -> void {
            xtaro::parse::Serialize<Json, Value>::to(*this, v);
        }

    private:
        std::string doToString(std::size_t indent = 0, bool hasName = false, bool isObjectValue = false) const;
        void ensureTyep(JsonType type) const;
        auto checkType(JsonType type) const -> bool;
        auto typeIndex() const noexcept -> std::size_t;

    private:
        struct Null {};
        std::variant<
            Null,
            bool,
            int,
            double,
            std::string,
            std::vector<Json>,
            std::unordered_map<std::string, Json>
        > _value;
    };

}

template <>
struct xtaro::parse::Deserialize<xtaro::parse::Json, bool> {
    static void from(const xtaro::parse::Json& j, bool& value) {
        value = j.asBoolean();
    }
};

template <>
struct xtaro::parse::Deserialize<xtaro::parse::Json, int> {
    static void from(const xtaro::parse::Json& j, int& value) {
        value = j.asInteger();
    }
};

template <>
struct xtaro::parse::Deserialize<xtaro::parse::Json, double> {
    static void from(const xtaro::parse::Json& j, double& value) {
        value = j.asDecimal();
    }
};

template <>
struct xtaro::parse::Deserialize<xtaro::parse::Json, std::string> {
    static void from(const Json& j, std::string& value) {
        value = j.asString();
    }
};

template <>
struct xtaro::parse::Deserialize<xtaro::parse::Json, std::filesystem::path> {
    static void from(const xtaro::parse::Json& j, std::filesystem::path& value) {
        value = j.asString();
    }
};

template <>
struct xtaro::parse::Deserialize<xtaro::parse::Json, std::tuple<std::string, int>> {
    static void from(const xtaro::parse::Json& j, std::tuple<std::string, int>& value) {
        auto arr = j.asArray();
        std::get<0>(value) = arr[0].asString();
        std::get<1>(value) = arr[1].asInteger();
    }
};

template <>
struct xtaro::parse::Deserialize<xtaro::parse::Json, std::any> {
    static void from(const xtaro::parse::Json& j, std::any& value) {
        using enum JsonType;
        switch (j.type()) {
            case JsonType::Null: break;
            case JsonType::Boolean: value = j.asBoolean(); break;
            case JsonType::Integer: value = j.asInteger(); break;
            case JsonType::Decimal: value = j.asDecimal(); break;
            case JsonType::String:  value = j.asString(); break;
            default: throw std::runtime_error{"Can't to a std::any"};
        }
    }
};

template <typename Value>
struct xtaro::parse::Deserialize<xtaro::parse::Json, std::vector<Value>> {
    static void from(const xtaro::parse::Json& json, std::vector<Value>& value) {
        auto arr = json.asArray();
        for (auto& j : arr) {
            auto v = Value{};
            xtaro::parse::Deserialize<xtaro::parse::Json, Value>::from(j, v);
            value.emplace_back(std::move(v));
        }
    }
};


template <typename Value>
struct xtaro::parse::Deserialize<xtaro::parse::Json, std::unordered_map<std::string, Value>> {
    static void from(const Json& json, std::unordered_map<std::string, Value>& value) {
        auto& map = json.asObject();
        for (auto& [key, j] : map) {
            auto pair = value.emplace(key, Value{});
            xtaro::parse::Deserialize<xtaro::parse::Json, Value>::from(j, pair.first->second);
        }
    }
};

template <typename Value>
struct xtaro::parse::Deserialize<xtaro::parse::Json, std::map<std::string, Value>> {
    static void from(const Json& json, std::map<std::string, Value>& value) {
        auto& map = json.asObject();
        for (auto& [key, j] : map) {
            auto pair = value.emplace(key, Value{});
            xtaro::parse::Deserialize<xtaro::parse::Json, Value>::from(j, pair.first->second);
        }
    }
};

template <typename Key, typename Value>
struct xtaro::parse::Deserialize<xtaro::parse::Json, std::map<Key, Value>> {
    static void from(const Json& json, std::map<Key, Value>& value) {
        auto& map = json.asObject();
        for (auto& [key, j] : map) {
            auto keyv = Key::fromString(key);
            auto pair = value.emplace(keyv, Value{});
            xtaro::parse::Deserialize<xtaro::parse::Json, Value>::from(j, pair.first->second);
        }
    }
};