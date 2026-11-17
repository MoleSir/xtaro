#include "replicatedbitcellarray.hh"
#include "circuit/base/port.hh"
#include "circuit/geometry/coord.hh"

#include <circuit/module/bitcellarray.hh>
#include <circuit/module/replicabitcellarray.hh>

#include <circuit/mod.hh>
#include <tech/tech.hh>

#include <ranges>
#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string ReplicatedBitcellArray::Arguments::toString() const
    {
        return std::format("r{}_c{}", this->rowSize, this->columnSize);
    }

    auto ReplicatedBitcellArray::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<ReplicatedBitcellArray>(circuitName, *this);
    }

    ReplicatedBitcellArray::ReplicatedBitcellArray(std::string_view name, const ReplicatedBitcellArray::Arguments& arguments) :
        Module{name},
        _rowSize{arguments.rowSize},
        _columnSize{arguments.columnSize}
    {
        debug::checkf(this->_rowSize >= 1, "Row size '{}' < 1", this->_rowSize);
        debug::checkf(this->_columnSize >= 1, "Column size '{}' < 1", this->_columnSize);
    
        this->createSelf();
    }
        

    void ReplicatedBitcellArray::createPorts()
    {
        // 'bl' and 'br'
        std::vector<std::string_view> blbrPortsName;
        for (auto i : std::ranges::views::iota(0, this->_columnSize))
            blbrPortsName.emplace_back( BITLINE_NAME(i) );
        for (std::int32_t i = 0; i < this->_columnSize; ++i)
            blbrPortsName.emplace_back( BITLINE_BAR_NAME(i) );

        this->addPortNets(blbrPortsName, PortType::InOut);

        // 'WL' ports
        std::vector<std::string_view> wlPortsName;
        for (std::int32_t i = 0; i < this->_rowSize; ++i)
            wlPortsName.emplace_back( WORDLINE_NAME(i) );
        
        this->addPortNets(wlPortsName, PortType::Input);

        // rbl
        this->addPort(REPLICA_BITLINE_NAME, PortType::InOut);
        this->addPort(REPLICA_BITLINE_BAR_NAME, PortType::Input);
        this->addPort(REPLICA_WORDLINE_NAME, PortType::Input);

        // 'vdd' and 'gnd'
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void ReplicatedBitcellArray::createCircuits()
    {   
        this->_bitcellArray = this->addCircuit(BitcellArray::Arguments{this->_rowSize, this->_columnSize});
        this->_replicaArray = this->addCircuit(ReplicaBitcellArray::Arguments{this->_rowSize});
    } 

    void ReplicatedBitcellArray::createInstances() 
    {
        // 'bl' and 'br'
        std::vector<std::string_view> bitcellarrayNes;
        for (auto i : std::ranges::views::iota(0, this->_columnSize))
            bitcellarrayNes.emplace_back( BITLINE_NAME(i) );
        for (std::int32_t i = 0; i < this->_columnSize; ++i)
            bitcellarrayNes.emplace_back( BITLINE_BAR_NAME(i) );

        // 'WL' ports
        for (std::int32_t i = 0; i < this->_rowSize; ++i)
            bitcellarrayNes.emplace_back( WORDLINE_NAME(i) );

        bitcellarrayNes.emplace_back(tech->vdd());
        bitcellarrayNes.emplace_back(tech->gnd());
        
        this->_bitcellArrayInst =
            this->addInstance(BitcellArray::TYPE, this->_bitcellArray, bitcellarrayNes);

        this->_replicaArrayInst = this->addInstance(
            ReplicaBitcellArray::TYPE, 
            this->_replicaArray, 
            {REPLICA_BITLINE_NAME, REPLICA_BITLINE_BAR_NAME, REPLICA_WORDLINE_NAME, tech->vdd(), tech->gnd()}
        );
    }

    void ReplicatedBitcellArray::calculateLayoutConstants()
    {
        this->_width = this->_bitcellArray->width() + this->_replicaArray->width();
        debug::checkf(
            this->_bitcellArray->height() == this->_replicaArray->height(), 
            "Bitcell array's height '{}' should equal to Replica array's height '{}'",
            this->_bitcellArray->height(),
            this->_replicaArray->height());
        this->_height = this->_bitcellArray->height();
    }

    void ReplicatedBitcellArray::placeInstances()
    {
        this->_bitcellArrayInst->place(Coord{0._u, 0._u});
        this->_replicaArrayInst->place(Coord{this->_bitcellArray->width(), 0._u});
    }

    void ReplicatedBitcellArray::routeInstances()
    {
    }
    
    void ReplicatedBitcellArray::createLayoutPins()
    {
        for (auto col : std::ranges::views::iota(0, this->_columnSize)) 
        {
            this->copyInstancePin(this->_bitcellArrayInst, BitcellArray::BITLINE_NAME(col), ReplicatedBitcellArray::BITLINE_NAME(col));
            this->copyInstancePin(this->_bitcellArrayInst, BitcellArray::BITLINE_BAR_NAME(col), ReplicatedBitcellArray::BITLINE_BAR_NAME(col));
        }
    
        for (auto row : std::ranges::views::iota(0, this->_rowSize))
            this->copyInstancePin(this->_bitcellArrayInst, BitcellArray::WORDLINE_NAME(row), ReplicatedBitcellArray::WORDLINE_NAME(row));
        
        this->copyInstancePin(this->_replicaArrayInst, ReplicaBitcellArray::REPLICA_BITLINE_NAME, ReplicatedBitcellArray::REPLICA_BITLINE_NAME);
        this->copyInstancePin(this->_replicaArrayInst, ReplicaBitcellArray::REPLICA_BITLINE_BAR_NAME, ReplicatedBitcellArray::REPLICA_BITLINE_BAR_NAME);
        this->copyInstancePins(this->_replicaArrayInst, ReplicaBitcellArray::REPLICA_WORDLINE_NAME, ReplicatedBitcellArray::REPLICA_WORDLINE_NAME);
    
        this->copyInstancePins(this->_bitcellArrayInst, tech->vdd());
        this->copyInstancePins(this->_bitcellArrayInst, tech->gnd());
    }

    auto ReplicatedBitcellArray::BITLINE_NAME(int index) -> std::string_view
    {
        return stringf("bl{}", index);
    }

    auto ReplicatedBitcellArray::BITLINE_BAR_NAME(int index) -> std::string_view
    {
        return stringf("br{}", index);
    }

    auto ReplicatedBitcellArray::WORDLINE_NAME(int index) -> std::string_view
    {
        return stringf("wl{}", index);
    }

}