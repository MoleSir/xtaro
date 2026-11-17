#include "nor.hh"

#include <circuit/component/mos.hh>

#include <circuit/mod.hh>

#include <utility/misc/string.hh>
#include <global/config/config.hh>
#include <global/tech/tech.hh>

#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string NOR::Arguments::toString() const
    {
        return std::format("{}_{}", this->inputSize, this->driveAbility);
    }

    auto NOR::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<NOR>(circuitName, *this);
    }

    NOR::NOR(std::string_view name, const NOR::Arguments& arguments) :
        Module{name},
        _driveAbility{arguments.driveAbility},
        _inputSize{arguments.inputSize},
        _nmos{nullptr},
        _pmos{nullptr}
    {
        debug::checkf(this->_inputSize >= 2, "Input size '{}' < 2", this->_inputSize);
        this->tryLoadFromLibrary();
    }

    void NOR::createPorts()
    {
        for (int i = 0; i < this->_inputSize; ++i)
            this->addPortNet(this->INPUT_NAME(i), PortType::Input);

        this->addPortNet(this->OUTPUT_NAME, PortType::Output);
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void NOR::createCircuits()
    {
        auto length = tech->layerWidth(Layer::Poly);

        // Assign arguments
        MOS::Arguments nmosArguments{MOSType::NMOS, length, length};
        MOS::Arguments pmosArguments{MOSType::PMOS, length, length};

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

    void NOR::createInstances() 
    {
        // PMOS
        std::vector<std::string_view> net{};
        for (int i = 0; i < this->_inputSize - 1; ++i)
            net.emplace_back( stringf("net{}", i) );

        for (int i = 0; i < this->_inputSize; ++i)
        {
            std::string_view name {stringf("nor_pmos{}", i)};
            if (i == 0)
                this->addInstance(name, this->_pmos, {this->OUTPUT_NAME, this->INPUT_NAME(i), net[0], tech->vdd()});
            else if (i == this->_inputSize - 1)
                this->addInstance(name, this->_pmos, {net.back(), this->INPUT_NAME(i), tech->vdd(), tech->vdd()});
            else
                this->addInstance(name, this->_pmos, {net[i - 1], this->INPUT_NAME(i), net[i], tech->vdd()});
        }
        
        // NMOS
        for (int i = 0; i < this->_inputSize; ++i)
            this->addInstance(stringf("nor_nmos{}", i), this->_nmos, {this->OUTPUT_NAME, this->INPUT_NAME(i), tech->gnd(), tech->gnd()});
    }

    void NOR::calculateLayoutConstants()
    {
        debug::unimplement("NOR::calculateLayoutConstants");
    }

    void NOR::placeInstances()
    {
        debug::unimplement("NOR::placeInstances");
    }

    void NOR::routeInstances()
    {
        debug::unimplement("NOR::placeInstances");
    }
    
    void NOR::createLayoutPins()
    {
        debug::unimplement("NOR::createLayoutPins");
    }

    auto NOR::INPUT_NAME(int index) -> std::string_view 
    {
        return stringf("A{}", index);
    }

}