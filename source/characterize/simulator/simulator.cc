#include "./simulator.hh"

#include "../command/meas.hh"
#include "global/config/config.hh"

#include <utility/misc/string.hh>
#include <utility/misc/sys.hh>
#include <utility/misc/file.hh>
#include <utility/debug/debug.hh>

#include <string>
#include <fstream>
#include <vector>
#include <memory>

namespace xtaro::characterize
{
    Simulator::Simulator(std::filesystem::path simulationFilename): 
        _simulationFilename{std::move(simulationFilename)},
        _simulationFile{}
    {
        this->_simulationFile.open(this->_simulationFilename);
        if (!this->_simulationFile.is_open())
            debug::exceptionfIn(
                "Generate spice simulate file",
                "Open '{}' failed!", this->_simulationFilename.c_str()
            );
    }

    Simulator::~Simulator() noexcept = default;

    void Simulator::writeContent(std::string_view content)
    {
        this->checkFileWritable();
        this->_simulationFile << content;
    }
    
    void Simulator::writeContent(char ch)
    {
        this->checkFileWritable();
        this->_simulationFile << ch;
    }

    void Simulator::writeInclude(const std::filesystem::path& filename)
    {
        this->checkFileWritable();;
        this->_simulationFile << ".include " << filename << '\n'; 
    }

    void Simulator::writeMeasurement(std::unique_ptr<Measurement> measurement)
    {
        measurement->writeCommand(this->_simulationFile);
        this->_measurements.emplace_back(std::move(measurement));
    }

    void Simulator::writeEnd()
    {
        this->checkFileWritable();
        this->_simulationFile << ".end\n";
    }

    void Simulator::writeComment(std::string_view comment)
    {
        this->checkFileWritable();
        this->_simulationFile << '*' << ' ' << comment << '\n';
    }

    void Simulator::writeTemperature(Number temperature)
    {
        this->checkFileWritable();
        this->_simulationFile << ".TEMP " << temperature.toString() << '\n';
    }

    void Simulator::writeInstance(
                    std::string_view moduleName,
                    std::string_view instanceName, 
                    const std::vector<std::string_view>& portsName)
    {
        this->checkFileWritable();

        this->_simulationFile << 'X' <<  instanceName;
        
        for (const std::string_view& portName : portsName)
            this->_simulationFile << ' ' << portName;
        
        this->_simulationFile << ' ' << moduleName << '\n';
    }

    void Simulator::writeSquareWaveVoltage(
                    std::string_view supplyName, 
                    std::string_view portName, 
                    const std::vector<Number>& times,
                    const std::vector<Number>& voltages,
                    Number slew)
    {
        this->checkFileWritable();

        if (times.size() != voltages.size())
            debug::exceptionfIn(
                "Write PWL volatage",
                "Time size '{}' != Voltage size '{}'", times.size(), voltages.size()
            );

        // Vclk clk 0 PWL (0n 0.0v 1n 0.0v 3n 5v 9n 5V 11n 0v 19n 0v 21n 5v) 
        this->_simulationFile << std::format("V{} {} 0 PWL (", supplyName, portName);
        
        if (times.size() == 0)
        {
            this->_simulationFile << ")\n";
            return;
        }

        this->_simulationFile << std::format("{} {} ", times[0], voltages[0]);

        for (std::size_t i = 1; i < times.size(); ++i)
        {
            this->_simulationFile << std::format("{} {} ", times[i] - slew, voltages[i - 1]);
            this->_simulationFile << std::format("{} {} ", times[i] + slew, voltages[i]);
        }

        this->_simulationFile << ")\n";
    }


    void Simulator::writeSquareWaveVoltage(
                    std::string_view voltageName, 
                    std::string_view netName, 
                    const std::map<Number, Number>& voltageMap,
                    Number slew
    )
    {
        std::vector<Number> times {};
        std::vector<Number> voltages {};

        for (const auto& [time, voltage] : voltageMap)
        {
            times.emplace_back(time);
            voltages.emplace_back(voltage);
        }

        this->writeSquareWaveVoltage(voltageName, netName, times, voltages, slew);
    }

    void Simulator::writePWLVoltage(
                    std::string_view supplyName, 
                    std::string_view portName, 
                    const std::vector<Number>& times,
                    const std::vector<Number>& voltages)
    {
        this->checkFileWritable();

        if (times.size() != voltages.size())
            debug::exceptionfIn(
                "Write PWL volatage",
                "Time size '{}' != Voltage size '{}'", times.size(), voltages.size()
            );

        // Vclk clk 0 PWL (0n 0.0v 1n 0.0v 3n 5v 9n 5V 11n 0v 19n 0v 21n 5v) 
        this->_simulationFile << std::format("V{} {} 0 PWL (", supplyName, portName);

        for (std::size_t i = 0; i < times.size(); ++i)
            this->_simulationFile << std::format("{} {} ", times[i], voltages[i]);

        this->_simulationFile << ")\n";
    }

    void Simulator::writePWLVoltage(
                    std::string_view voltageName, 
                    std::string_view netName,
                    const std::map<Number, Number>& voltageMap
    )
    {
        std::vector<Number> times {};
        std::vector<Number> voltages {};

        for (const auto& [time, voltage] : voltageMap)
        {
            times.emplace_back(time);
            voltages.emplace_back(voltage);
        }

        this->writePWLVoltage(voltageName, netName, times, voltages);
    }

    void Simulator::writePULSEVoltage(
                    std::string_view voltageName,
                    std::string_view netName, 
                    Number initVoltage,
                    Number pulsedVoltage,
                    Number delayTime,
                    Number riseTime,
                    Number fallTime,
                    Number pulseWidth,
                    Number period)
    {
        this->checkFileWritable();

        // Vclk0 clk0 0 PULSE (0 3.3 9.95n 0.1n 0.1n 4.9n 10n)
        this->_simulationFile << std::format(
            "V{} {} 0 PULSE({} {} {} {} {} {} {})\n",
            voltageName, netName,
            initVoltage, pulsedVoltage, 
            delayTime,
            riseTime, fallTime,
            pulseWidth, period
        );
    }

    void Simulator::writeDCVoltage(
                    std::string_view supplyName,
                    std::string_view portName,
                    Number voltage)
    {
        this->checkFileWritable();

        this->_simulationFile << std::format(
            "V{} {} 0 {}\n", 
            supplyName, 
            portName, 
            voltage
        );
    }

    void Simulator::writeTrans(Number stepTime, Number starTime, Number endTime)
    {
        this->checkFileWritable();

        this->_simulationFile << std::format(
            ".TRAN {} {} {}\n", 
            stepTime, 
            endTime,
            starTime
        );
    }

    void Simulator::writeCapacitance(
                    std::string_view name, 
                    std::string_view portName1, 
                    std::string_view protName2,
                    Number value)
    {
        this->checkFileWritable();
        
        // CD00 dout0_0 0 0f
        this->_simulationFile << std::format(
            "C{} {} {} {}\n", 
            name,
            portName1, 
            protName2, 
            value
        );
    }
    
    auto Simulator::simulate(const std::unique_ptr<Spicer>& spicer) -> std::unordered_map<std::string, Number> 
    {
        debug::debug("Run Simulate");
        auto resultPath = this->executeCommand(spicer);
        return this->getResults(resultPath);
    }

    auto Simulator::executeCommand(const std::unique_ptr<Spicer>& spicer) -> std::filesystem::path
    {
        this->_simulationFile.close();

        auto command 
            = spicer->simulationCommand(this->_simulationFilename, config->tempFolderPath);
        auto resultPath 
            = spicer->measurementFilePath(this->_simulationFilename, config->tempFolderPath);

        if (util::execute(command) != 0) {
            debug::exceptionf("Simulate '{}' failed...", command);
        }

        return resultPath;
    }

    std::unordered_map<std::string, Number> 
    Simulator::getResults(const std::filesystem::path& outputFilename)
    {
        // Read output file content
        std::string content = util::readFile(outputFilename);

        // Get each result
        std::unordered_map<std::string, Number> results {};
        for (auto& meas : this->_measurements)
        {
            // TODO: When failed...
            if (meas->getMeasureResult(content))
                results.emplace(meas->name(), meas->result());
        }

        return results;
    }

    void Simulator::checkFileWritable() const
    {
        if (!this->_simulationFile.is_open())
            debug::exceptionIn(
                "Write spice simulation",
                "The simulation file already closed."
            );
    }

}