#include "replicabitcellarray.hh"
#include "circuit/geometry/coord.hh"

#include <circuit/libmodule/bitcell.hh>
#include <circuit/libmodule/precharge.hh>
#include <circuit/libmodule/writedriver.hh>

#include <circuit/mod.hh>

#include <ranges>
#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string ReplicaBitcellArray::Arguments::toString() const
    {
        return std::format("bs{}", this->bitcellSize);
    }

    auto ReplicaBitcellArray::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<ReplicaBitcellArray>(circuitName, *this);
    }

    ReplicaBitcellArray::ReplicaBitcellArray(std::string_view name, const ReplicaBitcellArray::Arguments& arguments) :
        Module{name},
        _bitcellSize{arguments.bitcellSize}
    {
        debug::check(this->_bitcellSize >= 0, "Bitcell size < 0");
        debug::checkf(
            this->_bitcellSize >= LINKED_BITCELL_SIZE, 
            "Replica Bank's bitcell size '{}' < {}", this->_bitcellSize, static_cast<int>(LINKED_BITCELL_SIZE)
        );

        this->createSelf();
    }

    void ReplicaBitcellArray::createPorts()
    {
        this->addPortNet(ReplicaBitcellArray::REPLICA_BITLINE_NAME, PortType::InOut);
        this->addPortNet(ReplicaBitcellArray::REPLICA_BITLINE_BAR_NAME, PortType::InOut);
        this->addPortNet(ReplicaBitcellArray::REPLICA_WORDLINE_NAME, PortType::Input);
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void ReplicaBitcellArray::createCircuits()
    {   
        this->_bitcell = this->addCircuit(Bitcell::Arguments {});
    } 

    void ReplicaBitcellArray::createInstances() 
    {
        // Create bitcells
        for (int i = 0; i < this->_bitcellSize; ++i)
        {
            // bl br wl vdd gnd
            auto inst = this->addInstance(stringf("bicell{}", i), this->_bitcell, {
                REPLICA_BITLINE_NAME, REPLICA_BITLINE_BAR_NAME,
                i < LINKED_BITCELL_SIZE ? REPLICA_WORDLINE_NAME : tech->gnd(),
                tech->vdd(), tech->gnd()
            });
            this->_bitcellInsts.emplace_back(inst);
        }
    }

    void ReplicaBitcellArray::calculateLayoutConstants()
    {
        this->_width = this->_bitcell->width();
        this->_height = this->_bitcellSize * this->_bitcell->height();

        this->_bitlineLayer = this->_bitcell->getPin(Bitcell::BITLINE_NAME).layer();
        this->_wordlineLayer = this->_bitcell->getPin(Bitcell::WORDLINE_NAME).layer();
    }

    void ReplicaBitcellArray::placeInstances()
    {
        this->placeInstArrayInColumn(this->_bitcellInsts, 0._u, 0._u);
    }

    void ReplicaBitcellArray::routeInstances()
    {
        // MARK 
        auto blpin = this->_bitcellInsts.front()->getPin(Bitcell::BITLINE_NAME);
        auto brpin = this->_bitcellInsts.front()->getPin(Bitcell::BITLINE_BAR_NAME);

        this->addRectangle(this->_bitlineLayer, Coord{blpin.minX(), 0._u}, blpin.width(), this->_height);
        this->addRectangle(this->_bitlineLayer, Coord{brpin.minX(), 0._u}, brpin.width(), this->_height);

        for (auto i : std::ranges::views::iota(0, LINKED_BITCELL_SIZE - 1)) {
            auto wl1 = this->_bitcellInsts.at(i)->getPin(Bitcell::WORDLINE_NAME);
            auto wl2 = this->_bitcellInsts.at(i + 1)->getPin(Bitcell::WORDLINE_NAME);
            this->routePinsCenter(wl1, wl2, this->_bitlineLayer);
        }

        for (auto i : std::ranges::views::iota(LINKED_BITCELL_SIZE, this->_bitcellSize - 1)) {
            auto wl1 = this->_bitcellInsts.at(i)->getPin(Bitcell::WORDLINE_NAME);
            auto wl2 = this->_bitcellInsts.at(i + 1)->getPin(Bitcell::WORDLINE_NAME);
            this->routePinsCenter(wl1, wl2, this->_bitlineLayer);
        }
    }
    
    void ReplicaBitcellArray::createLayoutPins()
    {
        this->copyInstancePin(
            this->_bitcellInsts.front(), 
            Bitcell::BITLINE_NAME, 
            ReplicaBitcellArray::REPLICA_BITLINE_NAME);

        this->copyInstancePin(
            this->_bitcellInsts.front(), 
            Bitcell::BITLINE_BAR_NAME, 
            ReplicaBitcellArray::REPLICA_BITLINE_BAR_NAME);
    
        for (auto i : std::ranges::views::iota(0, LINKED_BITCELL_SIZE))
        {
            this->copyInstancePin(
                this->_bitcellInsts.at(i),
                Bitcell::WORDLINE_NAME,
                ReplicaBitcellArray::REPLICA_WORDLINE_NAME
        );
        }
    }

}