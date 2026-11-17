#pragma once

#include "./meas.hh"
#include <global/number/number.hh>
#include <string>
#include <fstream>

namespace xtaro::characterize
{

    enum class EdgeDirection
    {
        Fall = 0,
        Rise,
    };

    const char* edgeDirectionName(EdgeDirection direction) noexcept;

    class DelayMeasurement : public Measurement
    {
    public:
        DelayMeasurement(
            std::string name,
            std::string trigNetName, EdgeDirection trigDirection, Number trigVoltage, Number trigTimeDelay,
            std::string targNetName, EdgeDirection targDirection, Number targVoltage, Number targTimeDelay
        );

        virtual void writeCommand(std::ofstream& file) const override;

    private:
        std::string _trigNetName;
        EdgeDirection _trigDirection;
        Number _trigVoltage;
        Number _trigTimeDalay;

        std::string _targNetName;
        EdgeDirection _targDirection;
        Number _targVoltage;
        Number _targTimeDalay;
    };

}