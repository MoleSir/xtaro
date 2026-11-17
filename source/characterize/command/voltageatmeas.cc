#include "./voltageatmeas.hh"
#include <utility/misc/string.hh>

#include <string>
#include <fstream>
#include <format>

namespace xtaro::characterize
{
    VoltageAtMeasurement::VoltageAtMeasurement(
        std::string name, std::string netName, Number measureTime):
        Measurement{std::move(name)},
        _netName{std::move(netName)},
        _measureTime{measureTime}
    {
    }

    void VoltageAtMeasurement::writeCommand(std::ofstream& file) const
    {
        std::string command = std::format(
            ".meas tran {} FIND v({}) AT={}\n",
            this->_name,
            this->_netName,
            this->_measureTime
        );

        file << command;
    }
    
}