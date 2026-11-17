#include "function.hh"

#include "../command/voltageatmeas.hh"
#include "characterize/function/marchc/marchc.hh"
#include "characterize/function/marchcminus/marchcminus.hh"
#include "characterize/function/marchx/marchx.hh"
#include "characterize/function/mats/mats.hh"
#include "characterize/function/random/random.hh"
#include "characterize/simulator/sramsimulator.hh"
#include <cstdlib>
#include <global/number/number.hh>

#include <memory>
#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/debug/debug.hh>
#include <vector>

namespace xtaro::characterize
{

    auto FunctionTester::create(std::string_view policy, circuit::SRAM* sram, PVT pvt, SlewLoad slewload) -> std::unique_ptr<FunctionTester>
    {
        if (policy == "random")
            return std::make_unique<RandomTester>(sram, std::move(pvt), slewload);
        if (policy == "mats")
            return std::make_unique<MATSTester>(sram, std::move(pvt), slewload);
        if (policy == "marchx")
            return std::make_unique<MarchXTester>(sram, std::move(pvt), slewload);
        if (policy == "marchc")
            return std::make_unique<MarchCTester>(sram, std::move(pvt), slewload);
        if (policy == "marchcminus")
            return std::make_unique<MarchCMinusTester>(sram, std::move(pvt), slewload);

        debug::exceptionf("Un supported funection test policy '{}'", policy);
    }

    FunctionTester::FunctionTester(circuit::SRAM* sram, PVT pvt, SlewLoad slewload) :
        _simulator{std::make_unique<SRAMSimulator>(
            std::format(
                "{}/function_{}_{}.sp", 
                config->simFolderPath.c_str(),
                pvt.toString(),
                slewload.toString()
            ), 
            sram, 
            std::move(pvt),
            slewload
        )},
        
        _targetMeasResults{}
    {
    }

    bool FunctionTester::test()
    {        
        this->generateTransactions();
        this->writeTransactions();
        auto results = this->_simulator->simulate(Spicer::from(config->spiceSimulator));
        return this->checkSimluationReuslts(results);
    }

    void FunctionTester::addWriteTransaction(unsigned int address, unsigned int word)
    {
        this->_simulator->addWriteTransaction(address, word);
    }

    void FunctionTester::addReadTransaction(unsigned int address)
    {
        if (!this->_simulator->addReadTransaction(address))
            return;
        
        // Add measure 
        auto bits = this->_simulator->memory(address);
        auto measTime = this->_simulator->readMoment();

        // For each bit
        for (std::size_t bitIdx = 0; bitIdx < bits.size(); ++bitIdx)
        {
            bool bit = bits[bitIdx];
            Number targetResult {bit ? this->_simulator->vdd() : Number::zero()};

            std::size_t measIndex {this->_targetMeasResults.size()};
            std::string measName {std::format("dout{}_{}", bitIdx, measIndex)};
            std::string portName {std::format("dout{}", bitIdx)};

            this->_simulator->writeMeasurement(
                std::make_unique<VoltageAtMeasurement>(measName, portName, measTime)
            );

            this->_targetMeasResults.emplace(std::move(measName), targetResult);
        }
        this->_simulator->writeContent('\n');
    }

    void FunctionTester::writeTransactions()
    {
        debug::debug("Write Transactions");
        this->_simulator->writeTransactions();
    }

    bool FunctionTester::checkSimluationReuslts(const std::unordered_map<std::string, Number>& results) const
    {
        debug::debug("Check SimluationReuslts");

        std::size_t failedCount {0};
        for (const auto& kv : this->_targetMeasResults)
        {
            const std::string& measName {kv.first};
            auto iter {results.find(measName)};
            if (iter == results.end())
            {
                debug::errorf("The .MEAS task '{}' failed!", measName);
                return false;                
            }
            else
            {
                auto tartgetResult = kv.second;
                auto realResult = iter->second;
                if ( !FunctionTester::valueApproximate(tartgetResult, realResult) )
                    failedCount += 1;
            }
        }

        if (failedCount == 0) 
            debug::infof("Functional test pass: {} failed in {} test", failedCount, this->_targetMeasResults.size());
        else
            debug::warningf("Functional test failed: {} failed in {} test", failedCount, this->_targetMeasResults.size());
        return failedCount == 0;
    }

    bool FunctionTester::valueApproximate(const Number& n1, const Number& n2) 
    {
        constexpr auto deviation = 0.05;
        auto v1 = n1.toOrigin().value();
        auto v2 = n2.toOrigin().value();
        return std::abs(v1 - v2) < deviation;
    }
}