#include "inputdffs.hh"

#include <circuit/libmodule/dff.hh>

#include "../base/module.hh"
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/debug/debug.hh>
#include <utility/misc/string.hh>
#include <circuit/mod.hh>

namespace xtaro::circuit
{
 
    std::string InputDFFs::Arguments::toString() const
    {
        return std::format("aw{}_ww{}", this->addressWidth, this->wordWidth);
    }

    auto InputDFFs::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<InputDFFs>(circuitName, *this);
    }

    InputDFFs::InputDFFs(std::string_view name, const InputDFFs::Arguments& arguments) :
        Module{name},
        _addressWidth{arguments.addressWidth},
        _wordWidth{arguments.wordWidth}
    {
        debug::checkf(this->_addressWidth >= 1, "Address width '{}'", this->_addressWidth);
        debug::checkf(this->_wordWidth >= 1, "Word width '{}' < 1", this->_wordWidth < 1); 

        this->createSelf();
    }

    void InputDFFs::createPorts()
    {
        this->addPortNet("clk", PortType::Input);
        this->addPortNet("csb", PortType::Input);
        this->addPortNet("we", PortType::Input);

        for (int i = 0; i < this->_addressWidth; ++i)
            this->addPortNet(stringf("addr{}", i), PortType::Input);

        for (int i = 0; i < this->_wordWidth; ++i)
            this->addPortNet(stringf("din{}", i), PortType::Input);

        this->addPortNet("csb_r", PortType::Input);
        this->addPortNet("we_r", PortType::Input);

        for (int i = 0; i < this->_addressWidth; ++i)
            this->addPortNet(stringf("addr{}_r", i), PortType::Output);

        for (int i = 0; i < this->_wordWidth; ++i)
            this->addPortNet(stringf("din{}_r", i), PortType::Output);
        
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void InputDFFs::createCircuits()
    {
        this->_dff = this->addCircuit(DFF::Arguments {});
    } 

    void InputDFFs::createInstances() 
    {
        this->addInstance("we_dff", this->_dff, {"we", "we_r", "clk", tech->vdd(), tech->gnd()});
        this->addInstance("csb_dff", this->_dff, {"csb", "csb_r", "clk", tech->vdd(), tech->gnd()});

        for (int i = 0; i < this->_addressWidth; ++i)
            this->addInstance(
                stringf("addr{}_dff", i), this->_dff, {
                    stringf("addr{}", i), 
                    stringf("addr{}_r", i), 
                    "clk", tech->vdd(), tech->gnd()
                }
            );
    
        for (int i = 0; i < this->_wordWidth; ++i)
            this->addInstance(
                stringf("din{}", i), this->_dff, 
                {
                    stringf("din{}", i), 
                    stringf("din{}_r", i),
                    "clk", tech->vdd(), tech->gnd()
                }
            );
    }

    void InputDFFs::calculateLayoutConstants()
    {
        debug::unimplement("InputDFFs::calculateLayoutConstants");
    }

    void InputDFFs::placeInstances()
    {
        debug::unimplement("InputDFFs::placeInstances");
    }

    void InputDFFs::routeInstances()
    {
        debug::unimplement("InputDFFs::placeInstances");
    }

    void InputDFFs::createLayoutPins()
    {
        debug::unimplement("InputDFFs::createLayoutPins");
    }
    
}