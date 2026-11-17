#include "./delaymeas.hh"
#include <utility/misc/string.hh>
#include <format>

namespace xtaro::characterize
{

    const char* edgeDirectionName(EdgeDirection direction) noexcept
    {
        switch (direction)
        {
        case EdgeDirection::Fall: return "FALL";
        case EdgeDirection::Rise: return "RISE";
        }
        return nullptr;
    }

    DelayMeasurement::DelayMeasurement(
        std::string name,
        std::string trigNetName, EdgeDirection trigDirection, Number trigVoltage, Number trigTimeDelay,
        std::string targNetName, EdgeDirection targDirection, Number targVoltage, Number targTimeDelay
    ) :
        Measurement{std::move(name)},

        _trigNetName{std::move(trigNetName)}, 
        _trigDirection{trigDirection}, 
        _trigVoltage{trigVoltage}, 
        _trigTimeDalay{trigTimeDelay},
        
        _targNetName{std::move(targNetName)}, 
        _targDirection{targDirection}, 
        _targVoltage{targVoltage}, 
        _targTimeDalay{targTimeDelay}
    {
    }   
    
    // .meas tran delay_lh0 TRIG v(clk0) VAL=2.5 FALL=1 TD=0n TARG v(dout0_1) VAL=2.5 RISE=1 TD=0n
    void DelayMeasurement::writeCommand(std::ofstream& file) const
    {
        std::string command = 
            std::format(
                ".meas tran {} TRIG v({}) VAL={} {}=1 TD={} TARG v({}) VAL={} {}=1 TD={}\n",
                
                this->_name,
                
                this->_trigNetName,
                this->_trigVoltage,
                edgeDirectionName(this->_trigDirection),
                this->_trigTimeDalay,
            
                this->_targNetName,
                this->_targVoltage,
                edgeDirectionName(this->_targDirection),
                this->_targTimeDalay
            );
        file << command;
    }

}