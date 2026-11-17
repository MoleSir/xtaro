#include "./circuitsimulator.hh"
#include "./simulator.hh"
#include <global/number/number.hh>

#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>
#include <dump/mod.hh>

#include <parse/json/json.hh>

namespace xtaro::characterize
{

    CircuitSimulator::CircuitSimulator(
        std::filesystem::path simulationFilename, 
        circuit::Circuit* circuit, 
        PVT pvt
    ) :
        Simulator{std::move(simulationFilename)},
        _circuit{circuit},
        _pvt{std::move(pvt)}
    {
        // Is circuit output spice?
        if (!this->_circuit->hasSpiceFilePath()) 
            dump::writeSpice(this->_circuit);

        auto nmosModelPath = tech->nmosModelPath(this->_pvt.process);
        auto pmosModelPath = tech->pmosModelPath(this->_pvt.process);

        // Write include files
        this->writeContent('\n');
        this->writeInclude(nmosModelPath);
        this->writeInclude(pmosModelPath);
        this->writeInclude(this->_circuit->spiceFilePath());
        this->writeContent('\n');

        // Write enviroment
        this->writeDCVoltage(tech->vdd(), tech->vdd(), this->_pvt.voltage);
        this->writeTemperature(this->_pvt.temperature);
        this->writeContent('\n');

        // Write Instance
        this->writeInstance(this->_circuit->name(), this->_circuit->name(), this->_circuit->portsName());
        this->writeContent('\n');
    }

    void CircuitSimulator::writeLoads(Number load)
    {
        for (const std::string_view& portName : this->_circuit->outputPortsName())
            this->writeCapacitance(
                  portName,
                  portName,
                  "0", 
                  load);
        this->writeContent('\n');
    }

    void CircuitSimulator::writeClock(Number period, Number slew)
    {
        this->writePULSEVoltage(
            "clk", "clk", 
            this->_pvt.voltage, Number::origin(0.0),
            Number::origin(0.0), slew, slew,
            period / 2.0 - slew, period
        );
        this->writeContent('\n');
    }

    void CircuitSimulator::writeDCStimulation(std::string_view portName, Number voltage)
    {
        this->writeDCVoltage(portName, portName, voltage);
    }

    void CircuitSimulator::writePeriodStimulation(
        std::string_view portName, 
        const std::vector<Number>& voltages,
        Number period,
        Number slew,
        Number timeBias)
    {
        std::vector<Number> times {};
        for (std::size_t periodIdx = 0; periodIdx < voltages.size(); ++periodIdx)
        {
            auto time = (static_cast<double>(periodIdx) * period) + timeBias;
            times.emplace_back(
                time < Number::zero() ? Number::zero() : time
            );
        }
        this->writeSquareWaveVoltage(portName, portName, times, voltages, slew);
    }

    void CircuitSimulator::writeSquareWaveStimulation(
        std::string_view portName, 
        const std::map<Number, Number>& voltageMap,
        Number slew)
    {
        this->writeSquareWaveVoltage(portName, portName, voltageMap, slew);
    }

    void CircuitSimulator::writeSquareWaveStimulation(
        std::string_view portName,
        const std::vector<Number>& times,
        const std::vector<Number>& voltages,
        Number slew)
    {
        this->writeSquareWaveVoltage(portName, portName, times, voltages, slew);
    }

    void CircuitSimulator::writePWLStimulation(
        std::string_view portName, 
        const std::map<Number, Number>& voltageMap)
    {
        this->writePWLVoltage(portName, portName, voltageMap);
    }

    void CircuitSimulator::writePWLStimulation(
        std::string_view portName,
        const std::vector<Number>& times,
        const std::vector<Number>& voltages)
    {
        this->writePWLVoltage(portName, portName, times, voltages);
    }

}