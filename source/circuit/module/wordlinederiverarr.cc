#include "./wordlinederiverarr.hh"
#include "./wordlinederiver.hh"

#include <circuit/libmodule/and.hh>
#include <circuit/module/fanoutbuff.hh>

#include <circuit/mod.hh>
#include <tech/tech.hh>

#include <ranges>
#include <utility/misc/string.hh>
#include <utility/misc/math.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    std::string WordlineDriverArray::Arguments::toString() const
    {
        return std::format("ws{}", this->wlSize);
    }

    auto WordlineDriverArray::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<WordlineDriverArray>(circuitName, *this);
    }

    WordlineDriverArray::WordlineDriverArray(std::string_view name, const WordlineDriverArray::Arguments& arguments) :
        Module{name},
        _fanout{arguments.fanout},
        _wlSize{arguments.wlSize},
        _wordlineDriver{nullptr}
    {
        debug::checkf(this->_wlSize >= 1, "Wordline size '{}' < 1", this->_wlSize);   
        this->createSelf();
    }

    void WordlineDriverArray::createPorts()
    {
        for (int i = 0; i < this->_wlSize; ++i)
            this->addPortNet(WordlineDriverArray::WORDLINE_INPUT_NAME(i), PortType::Input);
            
        for (int i = 0; i < this->_wlSize; ++i)
            this->addPortNet(WordlineDriverArray::WORDLINE_OUTPUT_NAME(i), PortType::InOut);
        
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void WordlineDriverArray::createCircuits()
    {
        this->_wordlineDriver = this->addCircuit(WordlineDriver::Arguments {this->_fanout});
    } 

    void WordlineDriverArray::createInstances() 
    {
        for (int i = 0; i < this->_wlSize; ++i)
        {
            auto inst = this->addInstance(stringf("wordline_driver{}", i), this->_wordlineDriver, {
                WordlineDriverArray::WORDLINE_INPUT_NAME(i),
                WordlineDriverArray::WORDLINE_OUTPUT_NAME(i),
                tech->vdd(), tech->gnd()
            });
            this->_wordlineDriverInsts.emplace_back(inst);
        }
    }

    void WordlineDriverArray::calculateLayoutConstants()
    {
        this->_width = this->_wordlineDriver->width();
        this->_height = this->_wlSize * this->_wordlineDriver->height();
    }

    void WordlineDriverArray::placeInstances()
    {
        this->placeInstArrayInColumn(this->_wordlineDriverInsts, 0._u, 0._u);
    }

    void WordlineDriverArray::routeInstances()
    {
    }

    void WordlineDriverArray::createLayoutPins()
    {
        for (auto i : std::ranges::views::iota(0, this->_wlSize)) 
        {
            auto inst = this->_wordlineDriverInsts.at(i);
            this->copyInstancePin(inst,
                WordlineDriver::WORDLINE_INPUT_NAME, 
                WordlineDriverArray::WORDLINE_INPUT_NAME(i));
            this->copyInstancePin(inst,
                WordlineDriver::WORDLINE_OUTPUT_NAME, 
                WordlineDriverArray::WORDLINE_OUTPUT_NAME(i));

            this->copyInstancePin(inst, tech->vdd(), tech->vdd());
            this->copyInstancePin(inst, tech->gnd(), tech->gnd());
        }
    }

    auto WordlineDriverArray::WORDLINE_INPUT_NAME(int index) -> std::string_view 
    {
        return stringf("wl_in{}", index);
    }

    auto WordlineDriverArray::WORDLINE_OUTPUT_NAME(int index) -> std::string_view
    {
        return stringf("wl{}", index);
    }

}