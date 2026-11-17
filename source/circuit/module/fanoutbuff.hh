#pragma once

#include "../base/module.hh"

#include <deque>
#include <tuple>

namespace xtaro::circuit
{

    /*
        - Ports: in out0 out1 ... vdd gnd
    */
    class FanoutBuffer : public Module
    {
        enum 
        {
            MAX_FANOUT_SIZE = 1024,
        };

    public:
        static constexpr std::string_view TYPE = "fanout_buffer";

        /*
            - Construction: (int fanoutSize);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int fs) :
                Circuit::Arguments {FanoutBuffer::TYPE},
                fanoutSize{fs} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int fanoutSize;
        };

    public:
        FanoutBuffer(std::string_view name, const FanoutBuffer::Arguments& arguments);
        virtual ~FanoutBuffer() noexcept override = default;

    private:
        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
        
    private:
        using FanoutInfo = std::tuple<int, int, int>;
        // Use 'invSize' inv each has 'fanout' fanout to get 'outputSize' output net
        static auto calculateFanoutInfo(int outputSize) -> FanoutInfo;

        auto treeLevelInvSize(int level) const -> int;
        auto treeLevelFanout(int level) const -> int;

        auto treeLevelInvName(int level, int invIdx) const -> std::string_view;
        auto treeLevelInvInputName(int level, int invIdx) const -> std::string_view;
        auto treeLevelInvOutputName(int level, int invIdx) const -> std::string_view;

    private:
        int _fanoutSize;
        std::deque<FanoutInfo> _treeLevelFanoutInfos;

        Circuit* _inv;
    };

}