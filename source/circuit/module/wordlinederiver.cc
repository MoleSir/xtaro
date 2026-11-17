#include "./wordlinederiver.hh"
#include "circuit/base/module.hh"
#include "circuit/base/port.hh"
#include "circuit/geometry/coord.hh"
#include "circuit/module/buffer.hh"
#include "global/tech/driveability.hh"
#include "utility/debug/debug.hh"
#include <tech/tech.hh>
#include <format>
#include <memory>

namespace xtaro::circuit 
{

    std::string WordlineDriver::Arguments::toString() const {
        return std::format("fo{}", this->fanout);
    }

    auto WordlineDriver::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> {
        return std::make_unique<WordlineDriver>(circuitName, *this);
    }

    WordlineDriver::WordlineDriver(std::string_view name, const WordlineDriver::Arguments& arguments) :
        Module{name},
        _fanout{arguments.fanout}
    {
        debug::checkf(this->_fanout > 0, "Fanout size '{}' less than 1", this->_fanout);
        this->createSelf();
    }

    void WordlineDriver::createPorts() 
    {
        this->addPort(WordlineDriver::WORDLINE_INPUT_NAME, PortType::Input);
        this->addPort(WordlineDriver::WORDLINE_OUTPUT_NAME, PortType::Output);
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void WordlineDriver::createCircuits() 
    {
        auto dc = DriveAbility::X1;
        if (this->_fanout > 1) dc = DriveAbility::X1; 
        else if (this->_fanout >= 16) dc = DriveAbility::X2;
        else dc = DriveAbility::X3;

        this->_buffer = this->addCircuit(Buffer::Arguments{dc});
    }

    void WordlineDriver::createInstances() 
    {
        this->_bufferInst = this->addInstance(Buffer::TYPE, this->_buffer, {
            WordlineDriver::WORDLINE_INPUT_NAME, WordlineDriver::WORDLINE_OUTPUT_NAME, tech->vdd(), tech->gnd()
        });
    }

    void WordlineDriver::calculateLayoutConstants() 
    {
        this->_width = this->_buffer->width();
        this->_height = this->_buffer->height();
    }

    void WordlineDriver::placeInstances() 
    {
        this->_bufferInst->place(Coord{0._u, 0._u});
    }

    void WordlineDriver::routeInstances() 
    {
    }

    void WordlineDriver::createLayoutPins() 
    {
        this->copyInstancePins(this->_bufferInst, Buffer::INPUT_NAME, WordlineDriver::WORDLINE_INPUT_NAME);
        this->copyInstancePins(this->_bufferInst, Buffer::OUTPUT_NAME, WordlineDriver::WORDLINE_OUTPUT_NAME);
        this->copyInstancePin(this->_bufferInst, tech->vdd(), tech->vdd());
        this->copyInstancePin(this->_bufferInst, tech->gnd(), tech->gnd());
    }

}