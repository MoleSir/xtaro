#pragma once

#include <filesystem>
#include <memory>
#include <optional>

namespace xtaro::parse 
{

    /*
        A Deserializer should impl:
        - at(str)
        - get_to(v&)
        - static load_from(path) -> Deserializer
    */

    #define DE_FILED(f) dsr.at(#f).get_to(c.f);
    #define DE_OPTION_FILED(f) try { dsr.at(#f).get_to(c.f); } catch (const std::exception&) {}

    #define DE_FILED_AS(f, n) dsr.at(#n).get_to(c.f);
    #define DE_OPTION_FILED_AS(f, n) try { dsr.at(#n).get_to(c.f); } catch (const std::exception&) {}

    #define DE_VALUE_AS(o, n) if (s == n) { c = o; return; }

    #define DESERIALIZE_STRUCT(Struct, ...)\
    template <typename Deserializer>\
    struct xtaro::parse::Deserialize<Deserializer, Struct> {\
        static void from(const Deserializer& dsr, Struct& c) {\
            __VA_ARGS__\
        }\
    };

    #define DESERIALIZE_ENUM(Enum, ...)\
    template <typename Deserializer>\
    struct xtaro::parse::Deserialize<Deserializer, Enum> {\
        static void from(const Deserializer& dsr, Enum& c) {\
            using enum Enum;\
            auto s = std::string{};\
            Deserialize<Deserializer, std::string>::from(dsr, s);\
            __VA_ARGS__\
            std::runtime_error{std::format("'{}' is not an valid Enum literal", s)};\
        }\
    };

    template <typename Deserializer, typename Value>
    struct Deserialize {
        static void from(const Deserializer& dsr, Value& value) { static_assert(false, "Unimple"); }
    };

    template <typename Deserializer, typename Value>
    auto deserialize(const Deserializer& dsr) -> Value {
        auto value = Value{};
        Deserialize<Deserializer, Value>::from(dsr, value);
        return value;
    }

    template <typename Deserializer, typename Value>
    auto deserialize(const Deserializer& dsr, Value& value) -> void {
        Deserialize<Deserializer, Value>::from(dsr, value);
    }

    template <typename Deserializer, typename Value>
    auto deserializeFrom(const std::filesystem::path& path) -> Value {
        auto dsr = Deserializer::loadFromFile(path);
        return deserialize<Deserializer, Value>(dsr);
    }

    /*
        Default impl for useful data structure
    */

    template <typename Deserializer, typename Value>
    struct Deserialize<Deserializer, std::optional<Value>> {
        static void from(const Deserializer& dsr, std::optional<Value>& value) {
            auto inner = deserialize<Deserializer, Value>(dsr);
            value.emplace(std::move(inner));
        }
    };

    template <typename Deserializer, typename Value>
    struct Deserialize<Deserializer, std::unique_ptr<Value>> {
        static void from(const Deserializer& dsr, std::unique_ptr<Value>& value) {
            value = std::make_unique<Value>();
            Deserialize<Deserializer, Value>::from(dsr, *value);
        }
    };

}
