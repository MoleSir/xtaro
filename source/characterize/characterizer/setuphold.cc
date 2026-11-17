#include "./setuphold.hh"

#include "../simulator/circuitsimulator.hh"
#include "../command/delaymeas.hh"
#include <global/number/number.hh>

#include <circuit/libmodule/dff.hh>
#include <circuit/mod.hh>
#include <dump/mod.hh>

#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/debug/debug.hh>
#include <utility/misc/string.hh>

#include <cmath>
#include <memory>

namespace xtaro::characterize
{
    
    SetupHoldCharacterizer::SetupHoldCharacterizer(PVT pvt, Slews slews) :
        _dff{nullptr},
        _pvt{std::move(pvt)},
        _relatedSlew{slews.relatedSlew},
        _constrainedSlew{slews.constrainedSlew},
        _period{tech->normalPeriod()},

        _simulator{nullptr}
    {
        // Create DFF
        this->_dff = circuit::circuit(circuit::DFF::Arguments {});
        dump::writeSpice(this->_dff);
    }

    auto SetupHoldCharacterizer::analyze() -> SetupHoldResult
    {
        if (config->useSetupHoldModel) 
        {
            Number setup {tech->dffSetup()};
            Number hold {tech->dffHold()};
            return SetupHoldResult {
                .setupHL = setup,
                .setupLH = setup,
                .holdHL  = hold,
                .holdLH  = hold,
            };
        }

        return SetupHoldResult {
            .setupHL = this->getSetupHLTime(),
            .setupLH = this->getSetupLHTime(),
            .holdHL  = this->getHoldHLTime(),
            .holdLH  = this->getHoldHLTime(),
        };
    }

    auto SetupHoldCharacterizer::getTime(SamleType direction, TimeMode mode) -> Number
    {
        auto feasibleBound = Number::zero();
        auto infeasibleBound = Number::zero();

        if (mode == TimeMode::SETUP)
        {
            feasibleBound = 1.25 * this->_period;
            infeasibleBound = 2.5 * this->_period;
        }
        else
        {
            infeasibleBound = 1.5 * this->_period;
            feasibleBound = 2.75 * this->_period;
        }

        auto idealClkQ = this->simulate(direction, mode, feasibleBound);
        if (!idealClkQ.has_value())
            debug::exceptionIn("Ideal CLK-Q", "The init value not satisfy!"); // TODO
        auto setupholdTime = (feasibleBound - 2.0 * this->_period).abs();

        while (true)
        {
            Number dataArriveTime = (feasibleBound + infeasibleBound) / 2.0;
            debug::debugf("Data arrive time: {}", dataArriveTime);

            auto clkToQ = this->simulate(direction, mode, dataArriveTime);
            setupholdTime = (feasibleBound - 2.0 * this->_period).abs();     

            if (clkToQ.has_value() && clkToQ.value() < 1.1 * idealClkQ.value())
            {
                debug::debugf("PASS Clk-to-Q: {} Setup/Hold: {}", clkToQ.value(), setupholdTime);
                feasibleBound = dataArriveTime;
            }
            else
            {
                debug::debugf("FAIL Clk-to-Q: {} Setup/Hold: {}", clkToQ.value(), setupholdTime);
                infeasibleBound = dataArriveTime;
            }

            Number maxBound = infeasibleBound > feasibleBound ? infeasibleBound : feasibleBound;
            Number relativeRatio = (infeasibleBound - feasibleBound).abs() / maxBound.abs();
            debug::debugf("Relative ratio: {}", relativeRatio);
            if (relativeRatio.originValue() < 0.001)
                break;
        };

        return setupholdTime;
    }

    auto SetupHoldCharacterizer::simulate(
        SamleType direction, 
        TimeMode mode, 
        Number dataArriveTime) -> std::optional<Number>
    {
        const Number vdd { this->_pvt.voltage };

        // Create a _simulator
        this->_simulator = std::make_unique<CircuitSimulator>(
            config->simFolderPath / "setuphold.sp",
            this->_dff,
            this->_pvt
        );

        // Write CLK & D
        this->writeClock();
        this->writeD(direction, mode, dataArriveTime);

        auto meas = std::make_unique<DelayMeasurement>(
            "clk2q_delay",
            "clk", 
            EdgeDirection::Rise, 
            vdd / 2.0,
            1.9 * this->_period,
            "Q", 
            direction == RISE ? EdgeDirection::Rise : EdgeDirection::Fall,
            vdd / 2.0,
            1.9 * this->_period
        );
        this->_simulator->writeMeasurement(std::move(meas));

        // MARK
        this->_simulator->writeTrans(10.0_n, Number::zero(), 4. * this->_period);

        auto result = this->_simulator->simulate(Spicer::from(config->spiceSimulator));
        auto iter {result.find(meas->name())};
        if (iter != result.end())
            return std::optional<Number>{1e+9 * iter->second};
        return std::optional<Number>{};
    }

    void SetupHoldCharacterizer::writeD(
        SamleType direction, 
        TimeMode mode, 
        Number dataArriveTime)
    {
        const Number vdd { this->_pvt.voltage };
        auto correctValue   = direction == SamleType::RISE ? vdd : Number::zero();
        auto incorrectValue = direction == SamleType::FALL ? vdd : Number::zero(); 
    
        auto initValue = Number::zero();
        auto startValue = Number::zero();
        auto endValue = Number::zero();

        if (mode == TimeMode::SETUP) 
        {
            initValue = incorrectValue;
            startValue = incorrectValue;
            endValue = correctValue;
        }
        else
        {
            initValue = incorrectValue;
            startValue = correctValue;
            endValue = incorrectValue;
        }

        this->_simulator->writeSquareWaveStimulation(
            "D", 
            {Number::zero(), this->_period, dataArriveTime}, 
            {initValue, startValue, endValue}, 
            this->_constrainedSlew
        );
    }

    void SetupHoldCharacterizer::writeClock()
    {
        const auto vdd = this->_pvt.voltage;
        this->_simulator->writeSquareWaveStimulation(
            "clk", 
            {Number::zero(), 0.1 * this->_period, this->_period, 2. * this->_period},
            {Number::zero(), vdd, Number::zero(), vdd},
            this->_relatedSlew
        );
    }
}