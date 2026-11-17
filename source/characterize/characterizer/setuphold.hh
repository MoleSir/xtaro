#pragma once

#include <global/env/pvt.hh>
#include <global/env/slewload.hh>
#include <global/env/slews.hh>
#include <circuit/libmodule/dff.hh>
#include <optional>

namespace xtaro::characterize
{

    class CircuitSimulator;

    struct SetupHoldResult 
    {
        Number setupHL;
        Number setupLH;
        Number holdHL;
        Number holdLH;
    };

    class SetupHoldCharacterizer
    {

        enum SamleType
        {
            FALL = 0, RISE = 1,
        };

        enum TimeMode
        {
            SETUP = 0, HOLD = 1,
        };

    public:
        SetupHoldCharacterizer(PVT pvt, Slews slews);
        
    public:
        auto analyze() -> SetupHoldResult;

        auto getSetupLHTime() -> Number
        { return this->getTime(SamleType::RISE, TimeMode::SETUP); }
        
        auto getSetupHLTime() -> Number
        { return this->getTime(SamleType::FALL, TimeMode::SETUP); }
        
        auto getHoldLHTime() -> Number
        { return this->getTime(SamleType::RISE, TimeMode::HOLD); }

        auto getHoldHLTime() -> Number
        { return this->getTime(SamleType::FALL, TimeMode::HOLD); }

    private:
        auto getTime(SamleType direction, TimeMode mode) -> Number;
        auto simulate(SamleType direction, TimeMode mode, Number dataArriveTime) -> std::optional<Number>;

        void writeD(SamleType direction, TimeMode mode, Number dataArriveTime);
        void writeClock();

    private:
        circuit::Circuit* _dff;

        PVT _pvt;
        Number _relatedSlew;
        Number _constrainedSlew;
        Number _period;

        std::unique_ptr<CircuitSimulator> _simulator;
    };
}