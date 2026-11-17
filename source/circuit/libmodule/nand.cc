#include "nand.hh"

#include <circuit/component/mos.hh>
#include <global/config/config.hh>
#include <global/tech/tech.hh>

#include <circuit/mod.hh>

#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string NAND::Arguments::toString() const
    {
        return std::format("{}_{}", this->inputSize, this->driveAbility);
    }

    auto NAND::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<NAND>(circuitName, *this);
    }

    NAND::NAND(std::string_view name, const NAND::Arguments& arguments) :
        Module{name},
        _driveAbility{arguments.driveAbility},
        _inputSize{arguments.inputSize},
        _nmos{nullptr},
        _pmos{nullptr}
    {
        debug::checkf(this->_inputSize >= 2, "Input size '{}' < 2", this->_inputSize);

        this->tryLoadFromLibrary();
    }

    void NAND::createPorts()
    {
        for (int i = 0; i < this->_inputSize; ++i)
            this->addPortNet(this->INPUT_NAME(i), PortType::Input);

        this->addPortNet(this->OUTPUT_NAME, PortType::Output);
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void NAND::createCircuits()
    {
        auto length = tech->layerWidth(Layer::Poly);

        // Assign arguments
        MOS::Arguments nmosArguments {MOSType::NMOS, 0.0_o, length};
        MOS::Arguments pmosArguments {MOSType::PMOS, 0.0_o, length};

        switch (this->_driveAbility)
        {
        case DriveAbility::X0:
            nmosArguments.width = length;
            pmosArguments.width = length;
            break;
        case DriveAbility::X1:
            nmosArguments.width = 2.0 * length;
            pmosArguments.width = 4.0 * length;
            break;
        case DriveAbility::X2:
            nmosArguments.width = 4.0 * length;
            pmosArguments.width = 8.0 * length;
            break;
        case DriveAbility::X3:
            nmosArguments.width = 6.0 * length;
            pmosArguments.width = 12.0 * length;
            break;
        }

        // Create MOS
        this->_nmos = this->addCircuit(nmosArguments);
        this->_pmos = this->addCircuit(pmosArguments);
    } 

    void NAND::createInstances() 
    {
        // PMOS
        for (int i = 0; i < this->_inputSize; ++i)
            this->addInstance(stringf("nand_pmos{}", i), this->_pmos, {this->OUTPUT_NAME, this->INPUT_NAME(i), tech->vdd(), tech->vdd()});

        // NMOS
        std::vector<std::string_view> net{};
        for (int i = 0; i < this->_inputSize - 1; ++i)
            net.emplace_back( stringf("net{}", i) );

        for (int i = 0; i < this->_inputSize; ++i)
        {
            std::string_view name {stringf("nand_nmos{}", i)};
            if (i == 0)
                this->addInstance(name, this->_nmos, {this->OUTPUT_NAME, this->INPUT_NAME(i), net[0], tech->gnd()});
            else if (i == this->_inputSize - 1)
                this->addInstance(name, this->_nmos, {net.back(), this->INPUT_NAME(i), tech->gnd(), tech->gnd()});
            else
                this->addInstance(name, this->_nmos, {net[i - 1], this->INPUT_NAME(i), net[i], tech->gnd()});
        }
    }

    void NAND::calculateLayoutConstants()
    {
        debug::unimplement("NAND::calculateLayoutConstants");
    }

    void NAND::placeInstances()
    {
        debug::unimplement("NAND::placeInstances");
    }

    void NAND::routeInstances()
    {
        debug::unimplement("NAND::placeInstances");
    }

    void NAND::createLayoutPins()
    {
        debug::unimplement("NAND::createLayoutPins");
    }

    auto NAND::INPUT_NAME(int index) -> std::string_view 
    {
        return stringf("A{}", index);
    }

}