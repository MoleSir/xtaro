#include "buffer.hh"

#include "../base/module.hh"
#include "circuit/geometry/coord.hh"
#include "global/number/number.hh"
#include "global/tech/layer.hh"
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/debug/debug.hh>
#include <utility/misc/string.hh>

#include <circuit/libmodule/inv.hh>

namespace xtaro::circuit
{

    std::string Buffer::Arguments::toString() const
    {
        return std::format("{}", this->driveAbility);
    }

    auto Buffer::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<Buffer>(circuitName, *this);
    }

    Buffer::Buffer(std::string_view name, const Buffer::Arguments& arguments) :
        Module{name},
        _driveAbility{arguments.driveAbility}
    {
        this->createSelf();
    }

    void Buffer::createPorts()
    {
        this->addPortNet(Buffer::INPUT_NAME, PortType::Input);
        this->addPortNet(Buffer::OUTPUT_NAME, PortType::Input);
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void Buffer::createCircuits()
    {
        this->_inv = this->addCircuit(INV::Arguments {this->_driveAbility});
    } 

    void Buffer::createInstances() 
    {
        this->_invInst1 = this->addInstance("inv1", this->_inv, {Buffer::INPUT_NAME, "in_bar", tech->vdd(), tech->gnd()});
        this->_invInst2 = this->addInstance("inv2", this->_inv, {"in_bar", Buffer::OUTPUT_NAME, tech->vdd(), tech->gnd()});
    }

    void Buffer::calculateLayoutConstants()
    {
        this->_height = this->_inv->height();
        this->_width = 2 * this->_inv->width();
    }

    void Buffer::placeInstances()
    {
        this->_invInst1->place(Coord{0._u, 0._u});
        this->_invInst2->place(Coord{this->_invInst1->circuit()->width(), 0._u});
    }

    void Buffer::routeInstances()
    {
        auto pin1 = this->_invInst1->getPin(INV::OUTPUT_NAME);
        auto pin2 = this->_invInst2->getPin(INV::INPUT_NAME);
        this->routePinsCenter(pin1, pin2, Layer::Metal1);

        this->routePinsCenter(
            this->_invInst1->getPin(tech->vdd()), this->_invInst2->getPin(tech->vdd()),
            Layer::Metal1
        );

        this->routePinsCenter(
            this->_invInst1->getPin(tech->gnd()), this->_invInst2->getPin(tech->gnd()),
            Layer::Metal1
        );
    }

    void Buffer::createLayoutPins()
    {
        this->copyInstancePin(this->_invInst1, INV::INPUT_NAME, Buffer::INPUT_NAME);
        this->copyInstancePin(this->_invInst2, INV::OUTPUT_NAME, Buffer::OUTPUT_NAME);
        this->copyInstancePin(this->_invInst2, tech->vdd(), tech->vdd());
        this->copyInstancePin(this->_invInst2, tech->gnd(), tech->gnd());
    }
    
}