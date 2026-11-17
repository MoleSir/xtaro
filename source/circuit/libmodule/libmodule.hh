#pragma once

#include "../base/module.hh"
#include "circuit/base/circuit.hh"
#include <filesystem>
#include <global/tech/property.hh>
#include <optional>
#include <vector>

namespace xtaro::circuit
{

    class LibModule : public Module
    {
    public:
        static constexpr std::string_view TYPE = "lib_module";

        struct Arguments : public Circuit::Arguments 
        {
            Arguments(std::string mn, std::optional<ModuleProperty*> p, Module* pc) noexcept :
                Circuit::Arguments {LibModule::TYPE},
                moduleName{std::move(mn)},
                property{std::move(p)},
                parent{pc}
            {}
                
            virtual ~Arguments() noexcept = default;
            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            std::string moduleName;
            std::optional<ModuleProperty*> property;
            Module* parent;
        };

    public:
        LibModule(std::string_view name, const LibModule::Arguments& arguments);
        virtual ~LibModule() noexcept override = default;

    private:
        /// Read context in `path` and get module name and add port in `this`
        void loadNetlistInfoFromFile(const std::filesystem::path& path);

        /// Read `path` as `GDSIILayout`, set the width, height for `this`, and add pin in `this`
        void loadLayoutInfoFromFile(const std::filesystem::path& path);

        auto connectNets() const -> std::vector<std::string_view>;

    private:
        virtual void createPorts() override {}

        virtual void createCircuits() override {}
        virtual void createInstances() override {}

        virtual void calculateLayoutConstants() override {}
        virtual void placeInstances() override {}
        virtual void routeInstances() override {}
        virtual void createLayoutPins() override {}
        
    private:
        std::string _moduleName;
        std::optional<ModuleProperty*> _property;
        Module* _parent;
    };

}