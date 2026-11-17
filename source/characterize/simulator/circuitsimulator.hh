#pragma once

#include "./simulator.hh"

#include <circuit/base/circuit.hh>
#include <global/number/number.hh>
#include <global/env/pvt.hh>
#include <global/env/slewload.hh>
#include <global/env/slews.hh>

#include <string_view>
#include <map>

namespace xtaro::circuit
{
    class Circuit;
}

namespace xtaro::characterize
{

    class CircuitSimulator : public Simulator
    {
    public:
        CircuitSimulator(
            std::filesystem::path simulationFilePath,
            circuit::Circuit* circuit, 
            PVT pvt
        );
        
        void writeLoads(Number load);

        void writeClock(Number period, Number slew);

        void writeDCStimulation(std::string_view portName, Number voltage);

        void writePeriodStimulation(
             std::string_view portName, 
             const std::vector<Number>& voltages, 
             Number period,
             Number slew,
             Number timeBias);

        void writeSquareWaveStimulation(
             std::string_view portName, 
             const std::map<Number, Number>& voltageMap,
             Number slew);

        void writeSquareWaveStimulation(
             std::string_view portName,
             const std::vector<Number>& times,
             const std::vector<Number>& voltages,
             Number slew);
        
        void writePWLStimulation(
             std::string_view portName, 
             const std::map<Number, Number>& voltageMap);
             
        void writePWLStimulation(
             std::string_view portName,
             const std::vector<Number>& times,
             const std::vector<Number>& voltages);

    public:
        auto circuit() const noexcept -> circuit::Circuit*
        { return this->_circuit; }

        auto vdd() const noexcept -> Number
        { return this->_pvt.voltage; }

        auto pvt() const noexcept -> const PVT&
        { return this->_pvt; }

    protected:
        circuit::Circuit* _circuit;
        PVT _pvt;
    };

}