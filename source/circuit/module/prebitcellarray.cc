#include "./prebitcellarray.hh"

#include <circuit/module/bitcellarray.hh>
#include <circuit/module/andarray.hh>
#include <circuit/module/rowdecoder.hh>
#include <circuit/module/replicabank.hh>
#include <circuit/module/prechargearr.hh>
#include <circuit/module/senseamparr.hh>
#include <tech/tech.hh>
#include <circuit/mod.hh>

#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>

/*


    +---+  +-+
    |   |  | |
    |   |  | |
    |   |
    |   |
    |   |
    |   |
    |   |
    |   |
    |   |
    |   |
    |   |
    |   |
    +---+

*/

namespace xtaro::circuit
{
    std::string PerpheralBitcellArray::Arguments::toString() const
    {
        return std::format("rs{}_cs{}", this->rowSize, this->columnSize);
    }

    auto PerpheralBitcellArray::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<PerpheralBitcellArray>(circuitName, *this);
    }

    PerpheralBitcellArray::PerpheralBitcellArray(std::string_view name, const PerpheralBitcellArray::Arguments& arguments) :
        Module{name},

        _rowSize{arguments.rowSize},
        _columnSize{arguments.columnSize},
        
        _bitcellArray{nullptr},
        _replicaBank{nullptr},
        _prechargeArray{nullptr},
        _andArray{nullptr}
    {
        debug::checkf(this->_rowSize >= 1, "Row size '{}' < 1", this->_rowSize);
        debug::checkf(this->_columnSize >= 1, "Column size '{}' < 1", this->_columnSize);

        this->createSelf();
    }

    void PerpheralBitcellArray::createPorts()
    {
        this->addPortNet("wl_en", PortType::Input);
        this->addPortNet("p_en_bar", PortType::Input);
        this->addPortNet("we_en", PortType::Input);

        for (int i = 0; i < this->_columnSize; ++i)
            this->addPortNet( stringf("bl{}", i), PortType::InOut);

        for (int i = 0; i < this->_columnSize; ++i)
            this->addPortNet( stringf("br{}", i), PortType::InOut);

        for (int i = 0; i < this->_rowSize; ++i)
            this->addPortNet( stringf("rsel{}", i), PortType::InOut);

        this->addPortNet("rbl", PortType::InOut);

        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void PerpheralBitcellArray::createCircuits()
    {
        this->_replicaBank = 
            this->addCircuit(ReplicaBank::Arguments {this->_rowSize});

        this->_prechargeArray = 
            this->addCircuit(PrechargeArray::Arguments {this->_columnSize});

        BitcellArray::Arguments bitcellArrayArguments {this->_rowSize, this->_columnSize};
        this->_bitcellArray = this->addCircuit(bitcellArrayArguments);

        ANDArray::Arguments wldArrayArgs {this->_rowSize};
        this->_andArray = this->addCircuit(wldArrayArgs);
    }

    void PerpheralBitcellArray::createInstances() 
    {
        // Nets' name
        std::vector<std::string_view> blNets{};
        std::vector<std::string_view> brNets{};
        std::vector<std::string_view> rselNets{};
        std::vector<std::string_view> wlNets{};

        for (int i = 0; i < this->_columnSize; ++i)
        {
            blNets.emplace_back(stringf("bl{}", i));
            brNets.emplace_back(stringf("br{}", i));
        }
        
        for (int i = 0; i < this->_rowSize; ++i)
        {
            rselNets.emplace_back( stringf("rsel{}", i) );
            wlNets.emplace_back( stringf("wl{}", i) );
        }

        // Create and Connect replica bank
        // wl_en p_en_bar sa_en we_en rbl vdd gnd
        this->addInstance(ReplicaBank::TYPE, this->_replicaBank, {"wl_en", "p_en_bar", "we_en", "rbl", tech->vdd(), tech->gnd()});

        // Create and Connect precharge array
        // bl0 bl1 ... br0 br1 ... p_en_bar vdd gnd
        std::vector<std::string_view> prechargePorts{};
        for (auto& net : blNets)
            prechargePorts.emplace_back(net);
        for (auto& net : brNets)
            prechargePorts.emplace_back(net);
        prechargePorts.emplace_back("p_en_bar");
        prechargePorts.emplace_back(tech->vdd());
        prechargePorts.emplace_back(tech->gnd());
        this->addInstance(PrechargeArray::TYPE, this->_prechargeArray, prechargePorts);

        // Create and Connect bitcell array
        // Bitcell array' Ports: bl0 ... bln br0 ... brn ... wl0 ... wlm vdd gnd
        std::vector<std::string_view> bitcellarrPorts{};
        for (auto& net : blNets)
            bitcellarrPorts.emplace_back(net);
        for (auto& net : brNets)
            bitcellarrPorts.emplace_back(net);
        for (auto& net : wlNets)
            bitcellarrPorts.emplace_back(net);
        bitcellarrPorts.emplace_back(tech->vdd());
        bitcellarrPorts.emplace_back(tech->gnd());
        this->addInstance(BitcellArray::TYPE, this->_bitcellArray, bitcellarrPorts);
    
        // Create and Connect and array
        // A0 A1 ... en Z0 Z1 ... vdd gnd
        std::vector<std::string_view> andPorts{};
        for (auto& net : rselNets)
            andPorts.emplace_back(net);
        andPorts.emplace_back("wl_en");
        for (auto& net : wlNets)
            andPorts.emplace_back(net);

        andPorts.emplace_back(tech->vdd());
        andPorts.emplace_back(tech->gnd());
        this->addInstance(ANDArray::TYPE, this->_andArray, andPorts);
    }

    void PerpheralBitcellArray::calculateLayoutConstants()
    {
        debug::unimplement("PerpheralBitcellArray::calculateLayoutConstants");
    }

    void PerpheralBitcellArray::placeInstances()
    {
        debug::unimplement("PerpheralBitcellArray::placeInstances");
    }

    void PerpheralBitcellArray::routeInstances()
    {
        debug::unimplement("PerpheralBitcellArray::placeInstances");
    }

    void PerpheralBitcellArray::createLayoutPins()
    {
        debug::unimplement("PerpheralBitcellArray::createLayoutPins");
    }

}