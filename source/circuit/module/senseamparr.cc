#include "senseamparr.hh"
#include <circuit/libmodule/bitcell.hh>

#include <cassert>
#include <circuit/libmodule/senseamp.hh>
#include <circuit/module/fanoutbuff.hh>

#include <circuit/mod.hh>

#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string SenseAmplifierArray::Arguments::toString() const
    {
        return std::format("cs{}_scs{}{}", 
            this->columnSize, 
            this->spareColumnSize,
            this->useFanoutBuffer ? "_buf" : "");
    }

    auto SenseAmplifierArray::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<SenseAmplifierArray>(circuitName, *this);
    }

    SenseAmplifierArray::SenseAmplifierArray(std::string_view name, const SenseAmplifierArray::Arguments& arguments) :
        Module{name},
        _columnSize{arguments.columnSize},
        _spareColumnSize{arguments.spareColumnSize},
        _useFanoutBuffer{arguments.useFanoutBuffer}
    {
        debug::checkf(this->_columnSize >= 1, "Word width '{}' < 1", this->_columnSize);
        this->createSelf();
    }

    void SenseAmplifierArray::createPorts()
    {
        for (int i = 0; i < this->_columnSize; ++i)
            this->addPortNet(this->BITLINE_NAME(i), PortType::Input);

        for (int i = 0; i < this->_columnSize; ++i)
            this->addPortNet(this->BITLINE_BAR_NAME(i), PortType::Input);

        for (int i = 0; i < this->_columnSize; ++i)
            this->addPortNet(this->DATA_OUTPUT_NAME(i), PortType::Output);
        
        this->addPortNet(this->ENABLE_NAME, PortType::Input);
        
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void SenseAmplifierArray::createCircuits()
    {
        this->_senseamp = this->addCircuit(SenseAmplifier::Arguments {});
        
        if (this->_useFanoutBuffer)
        {
            FanoutBuffer::Arguments fanoutArguments {this->_columnSize};
            this->_fanoutbuf = this->addCircuit(fanoutArguments);
        }
    } 

    void SenseAmplifierArray::createInstances() 
    {
        if (this->_useFanoutBuffer)
        {
            assert(this->_fanoutbuf != nullptr);

            std::vector<std::string_view> fanoutNets {};
            fanoutNets.emplace_back(this->ENABLE_NAME);
            for (int i = 0; i < this->_columnSize; ++i)
                fanoutNets.emplace_back(stringf("sa_en_{}", i));
            fanoutNets.emplace_back(tech->vdd());
            fanoutNets.emplace_back(tech->gnd());

            this->addInstance(FanoutBuffer::TYPE, this->_fanoutbuf, fanoutNets);

            for (int i = 0; i < this->_columnSize; ++i)
            {
                auto inst = this->addInstance(stringf("senseamp{}", i), this->_senseamp, {
                    this->BITLINE_NAME(i), 
                    this->BITLINE_BAR_NAME(i), 
                    this->DATA_OUTPUT_NAME(i),
                    fanoutNets[i + 1],
                    tech->vdd(), tech->gnd()
                });
                this->_senseampInsts.emplace_back(inst);
            }
        }
        else 
        {
            assert(this->_fanoutbuf == nullptr);

            for (int i = 0; i < this->_columnSize; ++i)
            {
                auto inst = this->addInstance(stringf("senseamp{}", i), this->_senseamp, {
                    this->BITLINE_NAME(i), 
                    this->BITLINE_BAR_NAME(i), 
                    this->DATA_OUTPUT_NAME(i),
                    ENABLE_NAME,
                    tech->vdd(), tech->gnd()
                });
                this->_senseampInsts.emplace_back(inst);
            }    
        }
    }

    void SenseAmplifierArray::calculateLayoutConstants()
    {
        if (this->_useFanoutBuffer) {
            assert(this->_fanoutbuf != nullptr);
            debug::warning("Fanout Buffer in ANDArray does not support to build layout!");
        }

        auto bitcell = circuit(Bitcell::Arguments{});
        this->_intervalX = 
            (this->_spareColumnSize == 0) ?
            this->_senseamp->width() :
            (this->_spareColumnSize + 1) * bitcell->width();
        
        this->_height = this->_senseamp->height();
        this->_width = this->_intervalX * this->_columnSize;
    }

    void SenseAmplifierArray::placeInstances()
    {
        auto coord = Coord{0._u, 0._u};
        for (int i = 0; i < this->_columnSize; ++i) 
        {
            this->_senseampInsts[i]->place(coord);
            coord.x += this->_intervalX;
        }
    }

    void SenseAmplifierArray::routeInstances()
    {
        auto firstEnPin = this->_senseampInsts.front()->getPin(SenseAmplifier::ENABLE_NAME);
        auto lastEnPin = this->_senseampInsts.back()->getPin(SenseAmplifier::ENABLE_NAME);
        this->addLabelPin(
            firstEnPin.layer(), 
            SenseAmplifierArray::ENABLE_NAME,
            firstEnPin.leftdown(),
            lastEnPin.maxX() - firstEnPin.minX(),
            lastEnPin.height()
        );
    }
    
    void SenseAmplifierArray::createLayoutPins()
    {
        for (auto i = 0; i < this->_columnSize; ++i) {
            auto inst = this->_senseampInsts[i];

            this->copyInstancePin(inst, SenseAmplifier::DATA_OUTPUT_NAME, SenseAmplifierArray::DATA_OUTPUT_NAME(i));
            this->copyInstancePin(inst, SenseAmplifier::BITLINE_NAME, SenseAmplifierArray::BITLINE_NAME(i));
            this->copyInstancePin(inst, SenseAmplifier::BITLINE_BAR_NAME, SenseAmplifierArray::BITLINE_BAR_NAME(i));
            this->copyInstancePin(inst, tech->vdd(), tech->vdd());
            this->copyInstancePin(inst, tech->gnd(), tech->gnd());
        }
    }

    auto SenseAmplifierArray::BITLINE_NAME(int index) -> std::string_view 
    {
        return stringf("bl{}", index);
    }

    auto SenseAmplifierArray::BITLINE_BAR_NAME(int index) -> std::string_view
    {
        return stringf("br{}", index);
    }
    
    auto SenseAmplifierArray::DATA_OUTPUT_NAME(int index) -> std::string_view
    {
        return stringf("dout{}", index);
    }

}