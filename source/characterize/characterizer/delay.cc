#include "delay.hh"

#include "../command/delaymeas.hh"
#include "characterize/spicer/spicer.hh"
#include <global/number/number.hh>

#include <memory>
#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/debug/debug.hh>

#include <string>
#include <cstring>

namespace xtaro::characterize
{

    DelayCharacterizer::DelayCharacterizer(
            circuit::SRAM* sram, 
            PVT pvt,
            SlewLoad slewload) :
        _simulator{std::make_unique<SRAMSimulator>(
            std::format(
                "{}/delay_{}_{}.sp", 
                config->simFolderPath.c_str(),
                pvt.toString(),
                slewload.toString()
            ), 
            sram, 
            std::move(pvt),
            slewload
        )}
    {
    }

    auto DelayCharacterizer::analyze() -> DelayResult
    {
        auto addresses = this->generateRandomAddresses();

        this->writeTransactionsAndMeas(addresses);
        
        auto results = this->_simulator->simulate(Spicer::from(config->spiceSimulator));
        
        return this->calulateResult(results);
    }

    auto DelayCharacterizer::generateRandomAddresses() -> std::unordered_set<unsigned int>
    {
        int totalAddressCount {util::power(2, this->_simulator->sram()->addressWidth())};
        int addressCount = totalAddressCount * 0.1;
        if (addressCount == 0) addressCount = 2;

        std::unordered_set<unsigned int> addresses {};
        while (addressCount--)
            addresses.emplace( this->_simulator->randomAddress() );
        
        return addresses;
    }

    void DelayCharacterizer::writeTransactionsAndMeas(const std::unordered_set<unsigned int>& addresses)
    {
        for (unsigned int address : addresses)
        {
            this->addTransactions(address);
            this->writeMeas(address);
        }   

        this->_simulator->writeTransactions();
    }

    void DelayCharacterizer::addTransactions(unsigned int address)
    {
        this->_simulator->addWriteTransaction(address, 0U);
        this->_simulator->addReadTransaction(address);
    }

    void DelayCharacterizer::writeMeas(unsigned int address)
    {
        auto wordWidth = this->_simulator->sram()->wordWidth();
        for (int bit {0}; bit < wordWidth; ++bit)
        {
            auto outputPin = std::format("dout{}", bit);
            this->_simulator->writeMeasurement(std::make_unique<DelayMeasurement>(
                std::format("delay_hl_d{}_b{}", address, bit),
                "clk",
                EdgeDirection::Rise,
                this->_simulator->vdd() / 2.0,
                this->_simulator->readMoment() - this->_simulator->period(),
                outputPin,
                EdgeDirection::Fall,
                this->_simulator->vdd() / 2.0,
                this->_simulator->halfPeriodMoment()
            ));

            this->_simulator->writeMeasurement(std::make_unique<DelayMeasurement>(
                std::format("slew_hl_d{}_b{}", address, bit),
                outputPin,
                EdgeDirection::Fall,
                0.75 * this->_simulator->vdd(),
                this->_simulator->halfPeriodMoment(),
                outputPin,
                EdgeDirection::Fall,
                0.25 * this->_simulator->vdd(),
                this->_simulator->halfPeriodMoment()
            ));
        }
    }

    auto DelayCharacterizer::calulateResult(const std::unordered_map<std::string, Number>& results) -> DelayResult
    {
        std::vector<Number> delayHLs {};
        std::vector<Number> slewHLs {};

        for (auto& [name, value] : results) 
        {
            if (std::strncmp(name.c_str(), "delay_hl", 8) == 0) delayHLs.emplace_back(value);
            else if (std::strncmp(name.c_str(), "slew_hl", 7) == 0) slewHLs.emplace_back(value);
        }

        // MARK
        auto delayHL = 
            std::accumulate(delayHLs.begin(), delayHLs.end(), 0.0_o) / static_cast<double>(delayHLs.size()) ;
        auto slewHL =
            std::accumulate(slewHLs.begin(), slewHLs.end(), Number::zero()) / static_cast<double>(slewHLs.size()); 

        return DelayResult {
            .delayHL = delayHL,
            .delayLH = delayHL,
            .slewHL = slewHL,
            .slewLH = slewHL
        };
    }
    
}