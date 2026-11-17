#include "bitcellarray.hh"
#include "../base/module.hh"
#include "circuit/base/instance.hh"
#include "circuit/geometry/coord.hh"

#include <circuit/libmodule/bitcell.hh>

#include <circuit/mod.hh>

#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <ranges>
#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

#include <cmath>
#include <memory>
#include <cstdint>
#include <vector>

namespace xtaro::circuit
{
    /*
    
         +----------------------------+
     wln |                            |
         |                            |
                                      |
         .                            |
         .                            |
         .                            |
                                      |
     wl2 |                            |
         |                            |
     wl1 |                            |
         |                            |
     wl0 |                            |
         |                            |
         +----------------------------+
            bl0 bl1 bl2   ...     bln
    
    */      
    
    auto BitcellArray::Arguments::toString() const -> std::string
    {
        return std::format("r{}_c{}", this->rowSize, this->columnSize);
    }

    auto BitcellArray::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<BitcellArray>(circuitName, *this);
    }

    BitcellArray::BitcellArray(std::string_view name, const BitcellArray::Arguments& arguments) :
        Module{name},
        _rowSize{arguments.rowSize},
        _columnSize{arguments.columnSize}
    {
        debug::checkf(this->_rowSize >= 1, "Row size '{}' < 1", this->_rowSize);
        debug::checkf(this->_columnSize >= 1, "Column size '{}' < 1", this->_columnSize);

        this->createSelf();
    }

    void BitcellArray::createPorts()
    {
        // 'bl' and 'br'
        std::vector<std::string_view> blbrPortsName;
        for (std::int32_t i = 0; i < this->_columnSize; ++i)
            blbrPortsName.emplace_back( BitcellArray::BITLINE_NAME(i) );
        for (std::int32_t i = 0; i < this->_columnSize; ++i)
            blbrPortsName.emplace_back( BitcellArray::BITLINE_BAR_NAME(i) );

        this->addPortNets(blbrPortsName, PortType::InOut);

        // 'WL' ports
        std::vector<std::string_view> wlPortsName;
        for (std::int32_t i = 0; i < this->_rowSize; ++i)
            wlPortsName.emplace_back( BitcellArray::WORDLINE_NAME(i) );
        
        this->addPortNets(wlPortsName, PortType::Input);

        // 'vdd' and 'gnd'
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void BitcellArray::createCircuits()
    {
        this->_bitcell = this->addCircuit(Bitcell::Arguments {});
    }

    void BitcellArray::createInstances()
    {
        for (auto r : std::ranges::views::iota(0, this->_rowSize))
        {
            auto insts = std::vector<Instance*>{};
            for (auto c : std::ranges::views::iota(0, this->_columnSize))
            {                
                auto inst = this->addInstance(stringf("bitcell_{}_{}", r, c), this->_bitcell, {
                    BitcellArray::BITLINE_NAME(c),
                    BitcellArray::BITLINE_BAR_NAME(c),
                    BitcellArray::WORDLINE_NAME(r),
                    tech->vdd(),
                    tech->gnd()
                });
                insts.emplace_back(inst);
            }
            this->_bitcellInsts.emplace_back(std::move(insts));
        }
    }

    void BitcellArray::calculateLayoutConstants()
    {
        this->_width = this->_columnSize * this->_bitcell->width();
        this->_height = this->_rowSize * this->_bitcell->height();
    }

    void BitcellArray::placeInstances() 
    {
        for (std::int32_t r = 0; r < this->_rowSize; ++r)
        {
            auto mirror = r % 2 == 1 ? MirrorType::MirrorY : MirrorType::None;
            auto height = (r % 2 == 1 ? r + 1 : r) * this->_bitcell->height();
            for (std::int32_t c = 0; c < this->_columnSize; ++c)
            {
                auto inst = this->_bitcellInsts[r][c];
                auto coord = Coord{c * this->_bitcell->width(), height};
                inst->place(coord, mirror);
            }
        }
    }

    void BitcellArray::routeInstances()
    {
        for (auto col : std::ranges::views::iota(0, this->_columnSize)) 
        {
            auto inst = this->_bitcellInsts.at(0).at(col);

            auto blPin = inst->getPin(Bitcell::BITLINE_NAME);
            this->addRectangle(
                blPin.layer(), 
                blPin.leftdown().scale(1., 0.), 
                blPin.width(),
                this->_height);
            
            auto brPin = inst->getPin(Bitcell::BITLINE_BAR_NAME);
            this->addRectangle(
                brPin.layer(), 
                brPin.leftdown().scale(1., 0.), 
                brPin.width(),
                this->_height);
        }

        for (auto row : std::ranges::views::iota(0, this->_rowSize))
        {
            auto inst = this->_bitcellInsts.at(row).at(0);
            auto wlPin = inst->getPin(Bitcell::WORDLINE_NAME);
            this->addRectangle(
                wlPin.layer(), 
                wlPin.leftdown().scale(0., 1.), 
                this->_width, 
                wlPin.height()); 
        }
    }

    void BitcellArray::createLayoutPins()
    {
        // Create BL/BLB
        for (auto col : std::ranges::views::iota(0, this->_columnSize)) 
        {
            auto inst = this->_bitcellInsts.at(0).at(col);
            this->copyInstancePin(inst, Bitcell::BITLINE_NAME, BitcellArray::BITLINE_NAME(col));
            this->copyInstancePin(inst, Bitcell::BITLINE_BAR_NAME, BitcellArray::BITLINE_BAR_NAME(col));
        }
    
        // Create wordline
        for (auto row : std::ranges::views::iota(0, this->_rowSize))
        {
            auto inst = this->_bitcellInsts.at(row).at(0);
            this->copyInstancePin(inst, Bitcell::WORDLINE_NAME, BitcellArray::WORDLINE_NAME(row));
        }

        // Create VDD/GND
        // MARK: Bitcell: VDD higer GND
        /*
            ========= vdd
            |       |
            |       |
            |       |
            |       |
            ========= gnd        
        */
        for (auto row : std::ranges::views::iota(0, this->_rowSize) | 
                        std::ranges::views::filter([](auto n){ return n % 2 == 0; }))
        {
            auto inst = this->_bitcellInsts.at(row).at(0);
            this->copyInstancePin(inst, tech->vdd());
            this->copyInstancePin(inst, tech->gnd());
        }
        auto highestInst  = this->_bitcellInsts.back().at(0);
        this->copyInstancePin(highestInst, tech->gnd());
    }

    auto BitcellArray::BITLINE_NAME(int index) -> std::string_view
    {
        return stringf("bl{}", index);
    }
    
    auto BitcellArray::BITLINE_BAR_NAME(int index) -> std::string_view
    {
        return stringf("br{}", index);
    }
    
    auto BitcellArray::WORDLINE_NAME(int index) -> std::string_view
    {
        return stringf("wl{}", index);
    }

}