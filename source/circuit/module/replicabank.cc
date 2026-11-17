#include "replicabank.hh"

#include <circuit/libmodule/bitcell.hh>
#include <circuit/libmodule/precharge.hh>
#include <circuit/libmodule/writedriver.hh>

#include <circuit/mod.hh>

#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string ReplicaBank::Arguments::toString() const
    {
        return std::format("bs{}", this->bitcellSize);
    }

    auto ReplicaBank::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<ReplicaBank>(circuitName, *this);
    }

    ReplicaBank::ReplicaBank(std::string_view name, const ReplicaBank::Arguments& arguments) :
        Module{name},
        _bitcellSize{arguments.bitcellSize},
        _bitcell{nullptr},
        _prechage{nullptr},
        _writedriver{nullptr}
    {
        debug::check(this->_bitcellSize >= 0, "Bitcell size < 0");
        debug::checkf(
            this->_bitcellSize >= LINKED_BITCELL_SIZE, 
            "Replica Bank's bitcell size '{}' < {}", this->_bitcellSize, static_cast<int>(LINKED_BITCELL_SIZE)
        );

        this->createSelf();
    }

    void ReplicaBank::createPorts()
    {
        this->addPortNet("wl_en", PortType::Input);
        this->addPortNet("p_en_bar", PortType::Input);
        this->addPortNet("we_en", PortType::Input);

        this->addPortNet("rbl", PortType::InOut);
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void ReplicaBank::createCircuits()
    {   
        this->_bitcell     = this->addCircuit(Bitcell::Arguments {});
        this->_prechage    = this->addCircuit(Precharge::Arguments {});
        this->_writedriver = this->addCircuit(WriteDriver::Arguments {});
    } 

    void ReplicaBank::createInstances() 
    {
        // Create bitcells
        for (int i = 0; i < this->_bitcellSize; ++i)
        {
            // bl br wl vdd gnd
            this->addInstance(stringf("bicell{}", i), this->_bitcell, {
                "rbl", "rbr",
                i < LINKED_BITCELL_SIZE ? "wl_en" : tech->gnd(),
                tech->vdd(), tech->gnd()
            });
        }

        // Create outside circuits
        // bl br en_bar vdd gnd
        this->addInstance(Precharge::TYPE, this->_prechage, {"rbl", "rbr", "p_en_bar", tech->vdd()});

        // din bl br en vdd gnd
        this->addInstance(WriteDriver::TYPE, this->_writedriver, {tech->gnd(), "rbl", "rbr", "we_en", tech->vdd(), tech->gnd()});  
    }

    void ReplicaBank::calculateLayoutConstants()
    {
        debug::unimplement("ReplicaBank::calculateLayoutConstants");
    }

    void ReplicaBank::placeInstances()
    {
        debug::unimplement("ReplicaBank::placeInstances");
    }

    void ReplicaBank::routeInstances()
    {
        debug::unimplement("ReplicaBank::placeInstances");
    }
    
    void ReplicaBank::createLayoutPins()
    {
        debug::unimplement("ReplicaBank::createLayoutPins");
    }

}