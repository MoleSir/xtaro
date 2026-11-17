#include "prechargearr.hh"
#include "circuit/base/module.hh"

#include <circuit/libmodule/precharge.hh>
#include <circuit/module/fanoutbuff.hh>

#include <circuit/mod.hh>

#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string PrechargeArray::Arguments::toString() const
    {
        return std::format("cs{}{}", this->columnSize, this->useFanoutBuffer ? "_buf" : "");
    }

    auto PrechargeArray::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<PrechargeArray>(circuitName, *this);
    }

    PrechargeArray::PrechargeArray(std::string_view name, const PrechargeArray::Arguments& arguments) :
        Module{name},
        _columnSize{arguments.columnSize},
        _useFanoutBuffer{arguments.useFanoutBuffer}
    {
        debug::checkf(this->_columnSize >= 1, "Word width '{}' < 1", this->_columnSize);
        this->createSelf();
    }

    void PrechargeArray::createPorts()
    {
        for (int i = 0; i < this->_columnSize; ++i)
            this->addPortNet(this->BITLINE_NAME(i), PortType::InOut);

        for (int i = 0; i < this->_columnSize; ++i)
            this->addPortNet(this->BITLINE_BAR_NAME(i), PortType::InOut);
        
        this->addPortNet("p_en_bar", PortType::Input);
        
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void PrechargeArray::createCircuits()
    {
        this->_precharge = this->addCircuit(Precharge::Arguments {});
        if (this->_useFanoutBuffer)
            this->_fanoutbuf = this->addCircuit(FanoutBuffer::Arguments {this->_columnSize});
    } 

    void PrechargeArray::createInstances() 
    {
        if (this->_useFanoutBuffer)
        {
            assert(this->_fanoutbuf != nullptr);

            std::vector<std::string_view> fanoutNets {};
            fanoutNets.emplace_back(this->ENABLE_NAME);
            for (int i = 0; i < this->_columnSize; ++i)
                fanoutNets.emplace_back(stringf("we_en_{}", i));
            fanoutNets.emplace_back(tech->vdd());
            fanoutNets.emplace_back(tech->gnd());

            this->addInstance(FanoutBuffer::TYPE, this->_fanoutbuf, fanoutNets);

            for (int i = 0; i < this->_columnSize; ++i)
            {
                auto inst = this->addInstance(stringf("precharge{}", i), this->_precharge, {
                    this->BITLINE_NAME(i), 
                    this->BITLINE_BAR_NAME(i), 
                    fanoutNets[i + 1],
                    tech->vdd()
                });
                this->_prechargeInsts.emplace_back(inst);
            }
        }
        else
        {
            assert(this->_fanoutbuf == nullptr);

            for (int i = 0; i < this->_columnSize; ++i)
            {
                auto inst = this->addInstance(stringf("precharge{}", i), this->_precharge, {
                    this->BITLINE_NAME(i), 
                    this->BITLINE_BAR_NAME(i), 
                    ENABLE_NAME,
                    tech->vdd()
                });
                this->_prechargeInsts.emplace_back(inst);
            }
        }
    }

    void PrechargeArray::calculateLayoutConstants()
    {
        if (this->_useFanoutBuffer) {
            assert(this->_fanoutbuf != nullptr);
            debug::warning("Fanout Buffer in ANDArray does not support to build layout!");
        }

        this->_width = this->_columnSize * this->_precharge->width();
        this->_height = this->_precharge->height();
    }

    void PrechargeArray::placeInstances()
    {
        this->placeInstArrayInRow(this->_prechargeInsts, 0._u, 0._u);
    }

    void PrechargeArray::routeInstances()
    {
    }

    void PrechargeArray::createLayoutPins()
    {
        auto firstEnPin = this->_prechargeInsts.front()->getPin(Precharge::ENABLE_NAME);
        auto lastEnPin = this->_prechargeInsts.back()->getPin(Precharge::ENABLE_NAME);
        this->addLabelPin(
            firstEnPin.layer(), 
            PrechargeArray::ENABLE_NAME,
            firstEnPin.leftdown(),
            lastEnPin.maxX() - firstEnPin.minX(),
            lastEnPin.height()
        );

        for (auto i = 0; i < this->_columnSize; ++i) 
        {
            auto inst = this->_prechargeInsts[i];
            this->copyInstancePin(inst, Precharge::BITLINE_NAME, PrechargeArray::BITLINE_NAME(i));
            this->copyInstancePin(inst, Precharge::BITLINE_BAR_NAME, PrechargeArray::BITLINE_BAR_NAME(i));
            this->copyInstancePin(inst, tech->vdd(), tech->vdd());
        }
    }

    auto PrechargeArray::BITLINE_NAME(int index) -> std::string_view 
    {
        return stringf("bl{}", index);
    }

    auto PrechargeArray::BITLINE_BAR_NAME(int index) -> std::string_view 
    {
        return stringf("br{}", index);
    }

}