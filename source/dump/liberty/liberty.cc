#include "./liberty.hh"

#include <circuit/module/sram.hh>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

#include <fstream>
#include <cstdarg>
#include <sstream>
#include <vector>

namespace xtaro::dump
{

    Liberty::Liberty(
        circuit::SRAM* sram,
        PVT pvt,
        std::vector<Number> slews,
        std::vector<Number> loads,
        std::unordered_map<SlewLoad, characterize::DelayResult> delayResults,
        std::unordered_map<Slews, characterize::SetupHoldResult> setupholdResults,
        std::filesystem::path outputPath 
    ) :
        _sram{sram},
        _pvt{std::move(pvt)},

        _slews{std::move(slews)},
        _loads{std::move(loads)},
        
        _delayResults{std::move(delayResults)},
        _setupholdResults{std::move(setupholdResults)},

        _libfile{}
    {
        this->_libfile.open(outputPath);
    }

    void Liberty::write()
    {
        this->writeLibrary();
    }

    void Liberty::writeLibrary()
    {
        this->writeLinef(0, "library ({}_lib) {{", this->_sram->name());
        this->writeLine(1, "delay_model : \"table_lookup\";");

        this->writeUnits();
        this->writeDefaults();
        this->writeLUTTemplates();
        this->writeBus();

        this->writeCell();

        this->writeLine(0, '}'); // library
    }

    void Liberty::writeCell()
    {
        this->writeLinef(1, "cell ({}) {{", this->_sram->name());
        
        this->writeLine(2, "memory() {");
        this->writeLine(3, "type : ram;");
        this->writeLinef(3, "address_width : {};", this->_sram->addressWidth());
        this->writeLinef(3, "word_width : {};", this->_sram->wordWidth());
        this->writeLine(2, "}"); // memory
        this->writeEnter();

        this->writeLine(2, "interface_timing : true;");
        this->writeLine(2, "dont_use  : true;");
        this->writeLine(2, "map_only   : true;");
        this->writeLine(2, "dont_touch : true;");
        this->writeEnter();

        this->writePGPin();

        this->writeWordBus();

        this->writeAdderBus();

        this->writeControlPins();

        this->writeLine(1, '}'); // cell
    }

    void Liberty::writeUnits()
    {
        this->writeLine(1, "time_unit : \"1ns\";");
        this->writeLine(1, "voltage_unit : \"1V\"");
        this->writeLine(1, "current_unit : \"1mA\";");
        this->writeLine(1, "resistance_unit : \"1kohm\";");
        this->writeLine(1, "capacitive_load_unit(1, pF);");
        this->writeLine(1, "leakage_power_unit : \"1mW\";");
        this->writeLine(1, "pulling_resistance_unit :\"1kohm\";");
        this->writeLine(1, "operating_conditions(OC) {");
        this->writeLinef(2, "process : {};", this->_pvt.process);
        this->writeLinef(2, "voltage : {}", this->_pvt.voltage);
        this->writeLinef(2, "temperature : {}", this->_pvt.temperature);
        this->writeLine(1, "}");
        this->writeEnter();
    }

    void Liberty::writeDefaults()
    {
        this->writeLine(1, "input_threshold_pct_fall      : 50.0;");
        this->writeLine(1, "output_threshold_pct_fall     : 50.0;");
        this->writeLine(1, "input_threshold_pct_rise      : 50.0;");
        this->writeLine(1, "output_threshold_pct_rise     : 50.0;");
        this->writeLine(1, "slew_lower_threshold_pct_fall : 10.0;");
        this->writeLine(1, "slew_upper_threshold_pct_fall : 90.0;");
        this->writeLine(1, "slew_lower_threshold_pct_rise : 10.0;");
        this->writeLine(1, "slew_upper_threshold_pct_rise : 90.0;");
        
        // TODO nom
        this->writeLine(1, "default_cell_leakage_power    : 0.0;");
        this->writeLine(1, "default_leakage_power_density : 0.0;");
        this->writeLine(1, "default_input_pin_cap         : 1.0;");
        this->writeLine(1, "default_inout_pin_cap         : 1.0;");
        this->writeLine(1, "default_output_pin_cap        : 0.0;");
        this->writeLine(1, "default_max_transition        : 0.5;");
        this->writeLine(1, "default_fanout_load           : 1.0;");
        this->writeLine(1, "default_max_fanout            : 4.0;");
        this->writeLine(1, "default_connection_class      : universal;");

        this->writeLinef(1, "voltage_map (vdd, {});", this->_pvt.voltage);
        this->writeLine(1, "voltage_map (gnd, 0);");

        this->writeLine(1, "default_operating_conditions : OC;");

        this->writeEnter();
    }

    void Liberty::writeLUTTemplates()
    {
        this->writeLine(1, "lu_table_template(CELL_TABLE) {");
        this->writeLine(2, "variable_1 : input_net_transition;");
        this->writeLine(2, "variable_2 : total_output_net_capacitance;");
        this->writeIndex(1, this->_slews);
        this->writeIndex(2, this->_loads);
        this->writeLine(1, '}');
        
        this->writeEnter();

        this->writeLine(1, "lu_table_template(CONSTRAINT_TABLE) {");
        this->writeLine(2, "variable_1 : related_pin_transition;");
        this->writeLine(2, "variable_2 : constrained_pin_transition;");
        this->writeIndex(1, this->_slews);
        this->writeIndex(2, this->_slews);
        this->writeLine(1, '}');

        this->writeEnter();
    }

    void Liberty::writeBus()
    {
        this->writeLine(1, "type (data) {");
        this->writeLine(2, "base_type : array;");
        this->writeLine(2, "data_type : bit;");
        this->writeLinef(2, "bit_width : {};", this->_sram->wordWidth());
        this->writeLinef(2, "bit_from : {};", this->_sram->wordWidth() - 1);
        this->writeLine(2, "bit_to : 0;");
        this->writeLine(1, '}');
        this->writeEnter();

        this->writeLine(1, "type (addr) {");
        this->writeLine(2, "base_type : array;");
        this->writeLine(2, "data_type : bit;");
        this->writeLinef(2, "bit_width : {};", this->_sram->addressWidth());
        this->writeLinef(2, "bit_from : {};", this->_sram->addressWidth() - 1);
        this->writeLine(2, "bit_to : 0;");
        this->writeLine(1, '}');
        this->writeEnter();
    }

    void Liberty::writePGPin()
    {
        this->writeLine(2, "pg_pin(vdd) {");
        this->writeLine(3, "voltage_name : VDD;");
        this->writeLine(3, "pg_type : primary_power;");
        this->writeLine(2, '}');
        this->writeEnter();

        this->writeLine(2, "pg_pin(gnd) {");
        this->writeLine(3, "voltage_name : gnd;");
        this->writeLine(3, "pg_type : primary_ground;");
        this->writeLine(2, '}');
        this->writeEnter();
    }

    void Liberty::writeWordBus()
    {
        this->writeWordBusInput();
        this->writeWordBusOutput();
    }

    void Liberty::writeWordBusInput()
    {
        this->writeLine(2, "bus(din) {");
        this->writeLine(3, "bus_type  : data;");
        this->writeLine(3, "direction  : input;");
        this->writeLine(3, "memory_write() {");
        this->writeLine(4, "address : addr");
        this->writeLine(4, "clocked_on  : clk");
        this->writeLine(3, '}'); // memory_write
        this->writeLinef(3, "pin(din[{}:0]) {{", this->_sram->wordWidth() - 1);
        this->writeInputCharacter(4);
        this->writeLine(3, '}'); // pin
        this->writeLine(2, '}'); // bus

        this->writeEnter();
    }

    void Liberty::writeWordBusOutput()
    {
        this->writeLine(2, "bus(dout) {");
        this->writeLine(3, "bus_type  : data;");
        this->writeLine(3, "direction  : output;");
        // this->writeLine(3, "max_capacitance : ");
        // this->writeLine(3, "min_capacitance : ");
        this->writeLine(3, "memory_read() {");
        this->writeLine(4, "address : addr");
        this->writeLine(3, '}'); // memory_read()
        this->writeLinef(3, "pin(dout[{}:0]) {{", this->_sram->wordWidth() - 1);
        this->writeOutputCharacter(4);
        this->writeLine(3, '}'); // pin
        this->writeLine(2, '}'); // bus

        this->writeEnter();
    }

    void Liberty::writeAdderBus()
    {
        this->writeLine(2, "bus(addr) {");
        this->writeLine(3, "bus_type  : addr;");
        this->writeLine(3, "direction  : input;");
        // this->writeLine(2, "max_capacitance : ");
        // this->writeLine(2, "min_capacitance : ");
        this->writeLine(3, "memory_read() {");
        this->writeLine(3, "address : addr");
        this->writeLine(3, '}'); // memory_read()
        this->writeLinef(3, "pin(addr[{}:0]) {{", this->_sram->wordWidth() - 1);
        this->writeInputCharacter(4);
        this->writeLine(3, '}'); // pin
        this->writeLine(2, '}'); // bus

        this->writeEnter();
    }

    void Liberty::writeControlPins()
    {
        this->writeLine(2, "pin(csb) {");
        this->writeLine(3, "direction  : input;");
        this->writeInputCharacter(3);
        this->writeLine(2, '}'); // pin
        this->writeEnter();

        this->writeLine(2, "pin(we) {");
        this->writeLine(3, "direction  : input;");
        this->writeInputCharacter(3);
        this->writeLine(2, '}'); // pin
        this->writeEnter();

        this->writeLine(2, "pin(clk) {");
        this->writeLine(3, "direction  : input;");
        this->writeLine(2, '}'); // pin
        this->writeEnter();
    }

    void Liberty::writeIndex(int number, const std::vector<Number>& values)
    {
        std::stringstream ss {};

        ss << "index_" << number << '(' << '"';
        bool firstFlag {true};
        for (auto& v : values)
        {
            if (!firstFlag) ss << ',' << ' ';
            else firstFlag = false;
            ss << v.toNano().toString();
        }
        ss << '"' << ')';

        this->writeLine(2, ss.str());   
    }

    void Liberty::writeOutputCharacter(int indent)
    {
        /*
            timing(){ 
                timing_sense : non_unate; 
                related_pin : "clk"; 
                timing_type : falling_edge; 
                cell_rise(CELL_TABLE) {
                    ...
                }
                cell_fall(CELL_TABLE) {
                    ...
                }
                rise_transition(CELL_TABLE) {
                    ...
                }
                fall_transition(CELL_TABLE) {
                    ...
                }
            }
        */

        this->writeLine(indent, "timing() {");

        this->writeLine(indent + 1, "timing_sense : non_unate;");
        this->writeLine(indent + 1, "related_pin : \"clk\";");
        this->writeLine(indent + 1, "timing_type : rising_edge;");

        this->writeLine(indent + 1, "cell_rise(CELL_TABLE) {");
        this->writeDelayResult(indent + 2, &characterize::DelayResult::delayLH);
        this->writeLine(indent + 1, '}'); // cell_rise

        this->writeLine(indent + 1, "cell_fall(CELL_TABLE) {");
        this->writeDelayResult(indent + 2, &characterize::DelayResult::delayHL);
        this->writeLine(indent + 1, '}'); // cell_fall

        this->writeLine(indent + 1, "rise_transition(CELL_TABLE) {");
        this->writeDelayResult(indent + 2, &characterize::DelayResult::slewLH);
        this->writeLine(indent + 1, '}'); // rise_transition

        this->writeLine(indent + 1, "fall_transition(CELL_TABLE) {");
        this->writeDelayResult(indent + 2, &characterize::DelayResult::slewHL);
        this->writeLine(indent + 1, '}'); // fall_transition

        this->writeLine(indent, '}');  // timing
    }

    void Liberty::writeDelayResult(int indent, Number characterize::DelayResult::* attr)
    {
        /*
            values(\
                "0.006, 0.008, 0.015",\
                "0.006, 0.008, 0.015",\
                "0.006, 0.008, 0.015"\
            );
        */

        this->writeLine(indent, "values(\\");

        for (std::size_t slewIdx {0}; slewIdx < this->_slews.size(); ++slewIdx)
        {
            Number slew {this->_slews.at(slewIdx)};

            std::stringstream ss {};
            ss << '"';
            for (std::size_t loadIdx {0}; loadIdx < this->_loads.size(); ++loadIdx)
            {
                SlewLoad slewload {slew, this->_loads.at(loadIdx)};
                auto result {this->_delayResults.find(slewload)};
                if (result == this->_delayResults.end())
                    debug::exceptionIn("Write liberty", "No exit "); // TODO

                auto value = result->second.*attr;
                if (loadIdx != 0) ss << ", ";
                ss << value.toNano().toString();
            }
            ss << '"';

            if (slewIdx != this->_slews.size() - 1) ss << ',';
            ss << '\\';

            this->writeLine(indent + 1, ss.str());
        }

        this->writeLine(indent, ");");
    }

    void Liberty::writeInputCharacter(int indent)
    {
        /*
            timing(){ 
                timing_type : setup_rising; 
                related_pin  : "clk"; 
                rise_constraint(CONSTRAINT_TABLE) {
                    ...
                fall_constraint(CONSTRAINT_TABLE) {
                    ...
                }
            }
            timing(){ 
                timing_type : hold_rising; 
                related_pin  : "clk0"; 
                rise_constraint(CONSTRAINT_TABLE) {
                    ...
                }
                fall_constraint(CONSTRAINT_TABLE) {
                    ...
                }
            }
        */
        this->writeLine(indent, "timing() {");

        this->writeLine(indent + 1, "timing_type : setup_rising;");
        this->writeLine(indent + 1, "related_pin  : \"clk\";");

        this->writeLine(indent + 1, "rise_constraint(CONSTRAINT_TABLE) {");
        this->writeSetupHoldResult(indent + 2, &characterize::SetupHoldResult::setupLH);
        this->writeLine(indent + 1, '}');  // rise_constraint

        this->writeLine(indent + 1, "fall_constraint(CONSTRAINT_TABLE) {");
        this->writeSetupHoldResult(indent + 2, &characterize::SetupHoldResult::setupHL);
        this->writeLine(indent + 1, '}');  // rise_constraint

        this->writeLine(indent, '}');  // timing
        
        //////////////////////////////////////////////////////////

        this->writeLine(indent, "timing() {");

        this->writeLine(indent + 1, "timing_type : hold_rising;");
        this->writeLine(indent + 1, "related_pin  : \"clk\";");

        this->writeLine(indent + 1, "rise_constraint(CONSTRAINT_TABLE) {");
        this->writeSetupHoldResult(indent + 2, &characterize::SetupHoldResult::holdLH);
        this->writeLine(indent + 1, '}');  // rise_constraint

        this->writeLine(indent + 1, "fall_constraint(CONSTRAINT_TABLE) {");
        this->writeSetupHoldResult(indent + 2, &characterize::SetupHoldResult::holdHL);
        this->writeLine(indent + 1, '}');  // rise_constraint

        this->writeLine(indent, '}');  // timing
    }

    void Liberty::writeSetupHoldResult(int indent, Number characterize::SetupHoldResult::* attr)
    {
        /*
            values(\
                "0.006, 0.008, 0.015",\
                "0.006, 0.008, 0.015",\
                "0.006, 0.008, 0.015"\
            );
        */

        this->writeLine(indent, "values(\\");

        for (std::size_t relatedIdx {0}; relatedIdx < this->_slews.size(); ++relatedIdx)
        {
            auto related = this->_slews.at(relatedIdx);

            std::stringstream ss {};
            ss << '"';
            for (std::size_t constrainedIdx {0}; constrainedIdx < this->_slews.size(); ++constrainedIdx)
            {
                auto slews = Slews{related, this->_slews.at(constrainedIdx)};
                auto result = this->_setupholdResults.find(slews);
                if (result == this->_setupholdResults.end())
                    debug::exceptionIn("Write liberty", "No exit "); // TODO

                auto value = result->second.*attr;
                if (constrainedIdx != 0) ss << ", ";
                ss << value.toNano().toString();
            }
            ss << '"';

            if (relatedIdx != this->_slews.size() - 1) ss << ',';
            ss << '\\';

            this->writeLine(indent + 1, ss.str());
        }

        this->writeLine(indent, ");");
    }

    void Liberty::writeLine(int indent, std::string_view content)
    {
        while (indent--) this->_libfile << "    ";
        this->_libfile << content << '\n';
    }

    void Liberty::writeLine(int indent, char ch)
    {
        while (indent--) this->_libfile << "    ";
        this->_libfile << ch << '\n';
    }

    void Liberty::writeEnter()
    {
        this->_libfile << '\n';
    }
}