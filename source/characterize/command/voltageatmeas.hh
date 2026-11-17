#pragma once

#include "./meas.hh"
#include <global/number/number.hh>
#include <string>
#include <fstream>

namespace xtaro::characterize
{

    // .meas tran vdout0_0ck40 FIND v(dout0_0) AT=50.05n 
    class VoltageAtMeasurement : public Measurement
    {
    public:

    public:
        VoltageAtMeasurement(std::string name, std::string netName, Number measureTime);
        virtual void writeCommand(std::ofstream& file) const override;

    private:
        std::string _netName;
        Number _measureTime;
    };

}