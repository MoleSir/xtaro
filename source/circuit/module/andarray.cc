#include "./andarray.hh"
#include "circuit/geometry/coord.hh"
#include "global/tech/layer.hh"
#include <tech/tech.hh>

#include <cassert>
#include <circuit/libmodule/and.hh>
#include <circuit/module/fanoutbuff.hh>

#include <circuit/mod.hh>

#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string ANDArray::Arguments::toString() const
    {
        return std::format("as{}{}", this->andSize, this->useFanoutBuffer ? "_buf" : "");
    }

    auto ANDArray::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<ANDArray>(circuitName, *this);
    }

    ANDArray::ANDArray(std::string_view name, const ANDArray::Arguments& arguments) :
        Module{name},
        _andSize{arguments.andSize},
        _useFanoutBuffer{arguments.useFanoutBuffer}
    {
        debug::checkf(this->_andSize >= 1, "And size '{}' < 1", this->_andSize);   
        this->createSelf();
    }

    void ANDArray::createPorts()
    {
        for (int i = 0; i < this->_andSize; ++i)
            this->addPortNet(ANDArray::INPUT_NAME(i), PortType::Input);
            
        this->addPortNet("en", PortType::Input);

        for (int i = 0; i < this->_andSize; ++i)
            this->addPortNet(ANDArray::OUTPUT_NAME(i), PortType::InOut);
        
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void ANDArray::createCircuits()
    {
        this->_and = this->addCircuit(AND::Arguments {DriveAbility::X1, 2});
        
        if (this->_useFanoutBuffer) 
        {
            FanoutBuffer::Arguments fanoutArguments {this->_andSize};
            this->_fanoutbuf = this->addCircuit(fanoutArguments);
        }
    } 

    void ANDArray::createInstances() 
    {
        if (this->_useFanoutBuffer)
        {
            assert(this->_fanoutbuf != nullptr);

            std::vector<std::string_view> fanoutNets {};
            fanoutNets.emplace_back(ENABLE_NAME);
            for (int i = 0; i < this->_andSize; ++i)
                fanoutNets.emplace_back(stringf("en{}", i));
            fanoutNets.emplace_back(tech->vdd());
            fanoutNets.emplace_back(tech->gnd());

            this->addInstance(FanoutBuffer::TYPE, this->_fanoutbuf, fanoutNets);            
        
            for (int i = 0; i < this->_andSize; ++i)
            {
                auto inst = this->addInstance(stringf("and{}", i), this->_and, {
                    ANDArray::INPUT_NAME(i),
                    fanoutNets.at(i + 1),
                    ANDArray::OUTPUT_NAME(i), 
                    tech->vdd(), tech->gnd()
                });
                this->_andInsts.emplace_back(inst);
            }
        }
        else 
        {
            assert(this->_fanoutbuf == nullptr);

            for (int i = 0; i < this->_andSize; ++i)
            {
                auto inst = this->addInstance(stringf("and{}", i), this->_and, {
                    ANDArray::INPUT_NAME(i),
                    ENABLE_NAME,
                    ANDArray::OUTPUT_NAME(i), 
                    tech->vdd(), tech->gnd()
                });
                this->_andInsts.emplace_back(inst);
            }
        }
    }

    void ANDArray::calculateLayoutConstants()
    {
        if (this->_useFanoutBuffer) {
            assert(this->_fanoutbuf != nullptr);
            debug::warning("Fanout Buffer in ANDArray does not support to build layout!");
        }

        this->_width = this->_and->width();
        this->_height = this->_andSize * this->_and->height();
        this->_enableLayer = Layer::Metal2;
    }

    void ANDArray::placeInstances()
    {
        this->placeInstArrayInColumn(this->_andInsts, 0._u, 0._u);
    }

    void ANDArray::routeInstances()
    {
        // Route all input 1
        auto inst = this->_andInsts[0];
        auto pin = inst->getPin(AND::INPUT_NAME(1));
        this->addRectangle(Layer::Metal2, Coord{pin.minX(), 0._u}, pin.width(), this->_height);
    
        for (auto inst : this->_andInsts) 
        {
            auto pin = inst->getPin(AND::INPUT_NAME(1));
            this->addViaThroughtLayersCenter(pin.layer(), this->_enableLayer, pin.center());
        }
    }

    void ANDArray::createLayoutPins()
    {
        auto andPinName = AND::INPUT_NAME(1);
        for (auto i = 0; i < this->_andInsts.size(); ++i)
        {
            this->copyInstancePin(this->_andInsts[i], AND::INPUT_NAME(0), ANDArray::INPUT_NAME(i));
            this->copyInstancePin(this->_andInsts[i], AND::OUTPUT_NAME, ANDArray::OUTPUT_NAME(i));
        }

        this->copyInstancePin(this->_andInsts[0], AND::INPUT_NAME(1), ANDArray::ENABLE_NAME);
    }

    auto ANDArray::INPUT_NAME(int index) -> std::string_view
    {
        return stringf("A{}", index);
    }

    auto ANDArray::OUTPUT_NAME(int index) -> std::string_view
    {
        return stringf("Z{}", index);   
    }

}