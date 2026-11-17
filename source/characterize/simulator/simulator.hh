#pragma once

#include "../spicer/spicer.hh"
#include <global/number/number.hh>
#include <filesystem>
#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <map>

namespace xtaro::characterize
{
    class Measurement;

    /*
        Spice simulator class.
    */
    class Simulator
    {
    public:
        /*
            Simulator's contruction.
            - simulationFilename: The file where the spice commands are generated;
        */
        Simulator(std::filesystem::path simulationFilename);
        ~Simulator() noexcept;

        Simulator(const Simulator&) = delete;
        Simulator(Simulator&&) = delete;
        Simulator& operator = (const Simulator&) = delete;
        Simulator& operator = (Simulator&&) = delete;

    public:
        /*
            Write string to spice file
        */
        void writeContent(std::string_view content);
        void writeContent(char ch);
        
        /*
            Write a '.include' to command file. 
            - filename: The file need to include.
        */
        void writeInclude(const std::filesystem::path& filename);

        /*
            Write a '.END' spice command.
        */
        void writeEnd();

        /*
            Write spice 
        */
        void writeComment(std::string_view comment);
        
        /*
            Write a '.TEMP' command.
            - temperature: The temperature value.
        */
        void writeTemperature(Number temperature);

        /*
            Write a SUBCKT instance.

            e.g, write an INV Gate, whose name is "inv0", and it connects with "in", "in_bar", tech->vdd() and tech->gnd().
            Jsut call: `writeInstance("inv", "inv0", {"in", "in_bar", tech->vdd(), tech->gnd()});`.
            The nets in vector connect with ports vector of module by order.

            Method params:
            - moduleName: The module name of the instance belongs to. e.g "SRAM";
            - instanceName: The name of this instance. e.g "sram_16_2";
            - netsName: The name of the nets which connect with the ports of this instance.
              The net connects to port by order.
        */
        void writeInstance(
             std::string_view moduleName,
             std::string_view instanceName, 
             const std::vector<std::string_view>& netsName
        );

        /*
            Write a PWL Voltage source. 

            e.g: writeSquareWaveVoltage("CLK", "clk", {0, 1, 2}, {0, 3.3, 0}), 0.1);`
            Generate spice: VVCLK clk 0 PWL (0n 0.0v 0.95n 0v 1.05n 3.3v 1.95n 3.3v 2.05n 0v)

            Method params:
            - voltageName: The name of this voltage source.
            - netName: The net which voltage source connect to.
            - times: The time sequence of PWL.
            - voltages: The voltage sequence of PWL.
            - slew: The slew of the Voltage.
        */
        void writeSquareWaveVoltage(
             std::string_view voltageName, 
             std::string_view netName, 
             const std::vector<Number>& times,
             const std::vector<Number>& voltages,
             Number slew
        );

        void writeSquareWaveVoltage(
             std::string_view voltageName, 
             std::string_view netName, 
             const std::map<Number, Number>& voltageMap,
             Number slew
        );

        /*
            Write a PWL Voltage source. 
            But in this method, times and voltages write into spice directly.
        */
        void writePWLVoltage(
             std::string_view voltageName, 
             std::string_view netName, 
             const std::vector<Number>& times,
             const std::vector<Number>& voltages
        );

        void writePWLVoltage(
             std::string_view voltageName, 
             std::string_view netName,
             const std::map<Number, Number>& voltageMap
        );

        /*
            Write a PULSE Voltage source.
            - initVoltage: Initial value
            - pulsedVoltage: Pulsed value
            - delayTime: Delay time
            - riseTime: Rise time
            - fallTime: Fall time
            - pulseWidth: Pulse width
            - period: PER Period
        */
        void writePULSEVoltage(
             std::string_view voltageName,
             std::string_view netName, 
             Number initVoltage,
             Number pulsedVoltage,
             Number delayTime,
             Number riseTime,
             Number fallTime,
             Number pulseWidth,
             Number period 
        );

        /*
            Write a DC Voltage source.

            Method params:
            - voltageName: The name of this voltage source.
            - netName: The net which voltage source connect to.
            - voltage: DC voltage value.
        */
        void writeDCVoltage(
             std::string_view voltageName,
             std::string_view netName,
             Number voltage);
        
        /*
            Write a '.TRAN' command.

            .TRAN {stepTime}p {endTime}n {startTIme}n {stepTime}p
            
            Method params:
            - stepTime: Interval time of simulation.
            - beginTime: The time which begins to collect data.
            - endTime: Simulation ends time.
        */
        void writeTrans(Number stepTime, Number beginTime, Number endTime);

        /*
            Write a capacitance instance.

            C{capName} {net1Name} {net2Name} {capValue}f

            Method params:
            - capName: Name of capacitance.
            - net1Name: The name of net which connects which one side of CAP. 
            - net2Name: The name of net which connects which another side of CAP. 
            - value: Capacitance value.
        */
        void writeCapacitance(
             std::string_view capName, 
             std::string_view net1Name, 
             std::string_view net2tName,
             Number capValue
        );

        /*
            Write spice measurement.
        */
        void writeMeasurement(std::unique_ptr<Measurement> measurement);

    public:
        /*
            Run spice simulation. And return the result.
        */
        auto simulate(const std::unique_ptr<Spicer>& spicer) -> std::unordered_map<std::string, Number>;

    protected:
        /*
            Check the '_simulationFile' is writable.
            If it close, throw exception.
        */
        void checkFileWritable() const;

        auto executeCommand(const std::unique_ptr<Spicer>& spicer) -> std::filesystem::path;

        auto getResults(const std::filesystem::path& outputFilename) -> std::unordered_map<std::string, Number>;

    protected:
        std::filesystem::path _simulationFilename;
        std::ofstream _simulationFile;

        std::vector<std::unique_ptr<Measurement>> _measurements;
    };
};