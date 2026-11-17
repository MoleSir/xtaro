#include "writedriverarr.hh"
#include "circuit/geometry/coord.hh"
#include <circuit/libmodule/bitcell.hh>

#include <circuit/libmodule/writedriver.hh>
#include <circuit/module/fanoutbuff.hh>

#include <circuit/mod.hh>

#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string WriteDriverArray::Arguments::toString() const
    {
        return std::format("cs{}_scs{}{}", 
            this->columnSize, 
            this->spareColumnSize, 
            this->useFanoutBuffer ? "_buf" : "");
    }

    auto WriteDriverArray::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<WriteDriverArray>(circuitName, *this);
    }

    WriteDriverArray::WriteDriverArray(std::string_view name, const WriteDriverArray::Arguments& arguments) :
        Module{name},
        _columnSize{arguments.columnSize},
        _spareColumnSize{arguments.spareColumnSize},
        _useFanoutBuffer{arguments.useFanoutBuffer}
    {
        debug::checkf(this->_columnSize >= 1, "Word width '{}' < 1", this->_columnSize);
        this->createSelf();
    }

    void WriteDriverArray::createPorts()
    {
        for (int i = 0; i < this->_columnSize; ++i)
            this->addPortNet(this->DATA_INPUT_NAME(i), PortType::Input);

        for (int i = 0; i < this->_columnSize; ++i)
            this->addPortNet(this->BITLINE_NAME(i), PortType::InOut);

        for (int i = 0; i < this->_columnSize; ++i)
            this->addPortNet(this->BITLINE_BAR_NAME(i), PortType::InOut);
        
        this->addPortNet("we_en", PortType::Input);
        
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void WriteDriverArray::createCircuits()
    {
        this->_writedriver = this->addCircuit(WriteDriver::Arguments {});
        
        if (this->_useFanoutBuffer) 
        {
            FanoutBuffer::Arguments fanoutArguments {this->_columnSize};
            this->_fanoutbuf = this->addCircuit(fanoutArguments);
        }
    } 

    void WriteDriverArray::createInstances() 
    {
        if (this->_useFanoutBuffer) 
        {
            assert(this->_fanoutbuf != nullptr);

            std::vector<std::string_view> fanoutNets {};
            fanoutNets.emplace_back(ENABLE_NAME);
            for (int i = 0; i < this->_columnSize; ++i)
                fanoutNets.emplace_back(stringf("we_en_{}", i));
            fanoutNets.emplace_back(tech->vdd());
            fanoutNets.emplace_back(tech->gnd());

            this->addInstance(FanoutBuffer::TYPE, this->_fanoutbuf, fanoutNets);

            for (int i = 0; i < this->_columnSize; ++i)
            {
                auto inst = this->addInstance(stringf("write_driver{}", i), this->_writedriver, {
                    this->DATA_INPUT_NAME(i),
                    this->BITLINE_NAME(i), 
                    this->BITLINE_BAR_NAME(i),
                    fanoutNets.at(i + 1),
                    tech->vdd(), tech->gnd()
                });
                this->_writedriverInsts.emplace_back(inst);
            }
        }
        else
        {
            assert(this->_fanoutbuf == nullptr);
            for (int i = 0; i < this->_columnSize; ++i)
            {
                auto inst = this->addInstance(stringf("write_driver{}", i), this->_writedriver, {
                    this->DATA_INPUT_NAME(i),
                    this->BITLINE_NAME(i), 
                    this->BITLINE_BAR_NAME(i),
                    ENABLE_NAME,
                    tech->vdd(), tech->gnd()
                });
                this->_writedriverInsts.emplace_back(inst);
            }
        }
    }

    void WriteDriverArray::calculateLayoutConstants()
    {
        if (this->_useFanoutBuffer) {
            assert(this->_fanoutbuf != nullptr);
            debug::warning("Fanout Buffer in ANDArray does not support to build layout!");
        }

        auto bitcell = circuit(Bitcell::Arguments{});
        this->_intervalX = 
            (this->_spareColumnSize == 0) ?
            this->_writedriver->width() :
            (this->_spareColumnSize + 1) * bitcell->width();
        
        this->_height = this->_writedriver->height();
        this->_width = this->_intervalX * this->_columnSize;
    }

    void WriteDriverArray::placeInstances()
    {
        auto coord = Coord{0._u, 0._u};
        for (int i = 0; i < this->_columnSize; ++i) 
        {
            this->_writedriverInsts[i]->place(coord);
            coord.x += this->_intervalX;
        }
    }

    void WriteDriverArray::routeInstances()
    {
        auto firstEnPin = this->_writedriverInsts.front()->getPin(WriteDriver::ENABLE_NAME);
        auto lastEnPin = this->_writedriverInsts.back()->getPin(WriteDriver::ENABLE_NAME);
        this->addLabelPin(
            firstEnPin.layer(), 
            WriteDriverArray::ENABLE_NAME,
            firstEnPin.leftdown(),
            lastEnPin.maxX() - firstEnPin.minX(),
            lastEnPin.height()
        );
    }

    void WriteDriverArray::createLayoutPins()
    {
        for (auto i = 0; i < this->_columnSize; ++i) {
            auto inst = this->_writedriverInsts[i];

            this->copyInstancePin(inst, WriteDriver::DATA_INPUT_NAME, WriteDriverArray::DATA_INPUT_NAME(i));
            this->copyInstancePin(inst, WriteDriver::BITLINE_NAME, WriteDriverArray::BITLINE_NAME(i));
            this->copyInstancePin(inst, WriteDriver::BITLINE_BAR_NAME, WriteDriverArray::BITLINE_BAR_NAME(i));
            this->copyInstancePin(inst, tech->vdd(), tech->vdd());
            this->copyInstancePin(inst, tech->gnd(), tech->gnd());
        }
    }
    
    auto WriteDriverArray::BITLINE_NAME(int index) -> std::string_view {
        return stringf("bl{}", index);
    }

    auto WriteDriverArray::BITLINE_BAR_NAME(int index) -> std::string_view {
        return stringf("br{}", index);
    }

    auto WriteDriverArray::DATA_INPUT_NAME(int index) -> std::string_view {
        return stringf("din{}", index);
    }

}