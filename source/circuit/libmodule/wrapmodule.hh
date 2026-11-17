#pragma once

#include "../base/module.hh"
#include <string_view>

namespace xtaro::circuit
{

    class WrapModule : public Module
    {
    public:
        WrapModule(std::string_view name) : Module{name} {}
        virtual ~WrapModule() noexcept override = default;

    private:
        virtual void createCircuits() override {}
        virtual void createInstances() override {}

        virtual void calculateLayoutConstants() override {}
        virtual void placeInstances() override {}
        virtual void routeInstances() override {}
        virtual void createLayoutPins() override {}
    };

}