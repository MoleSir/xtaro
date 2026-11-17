#include "test.hh"
#include "../base/module.hh"
#include "global/tech/layer.hh"
#include <circuit/geometry/coord.hh>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/debug/debug.hh>

namespace xtaro::circuit
{
    Test::Test(std::string_view name, const Test::Arguments& arguments) :
        Module{name}
    {
        this->createSelf();
    }

    auto Test::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<Test>(circuitName, *this);
    }

    void Test::createPorts()
    {
        // in out en en_bar vdd gnd
        this->addPortNet("in1", PortType::Input);
        this->addPortNet("in2", PortType::Input);
        this->addPortNet("out1", PortType::Output);
        this->addPortNet("out2", PortType::Output);
        this->addPortNet(tech->vdd(), PortType::InOut);
        this->addPortNet(tech->gnd(), PortType::InOut);
    }

    void Test::createCircuits()
    {

    } 

    void Test::createInstances() 
    {

    }

    void Test::calculateLayoutConstants()
    {
    }

    void Test::placeInstances()
    {
        this->addRectangleCenter(Layer::Poly, {0._n, 0._n}, 50._n, 350._n);
        this->addRectangleCenter(Layer::Active, {0._n, 0._n}, 150._n, 250._n);
        this->addRectangleCenter(Layer::Active, {-150._n, 0._n}, 50._n, 250._n);
        this->addRectangleCenter(Layer::NWell, {-50._n, 0._n}, 400._n, 400._n);

        this->addRectangleCenter(Layer::NImplant, {0._n, 0._n}, 180._n, 375._n);
        this->addRectangleCenter(Layer::PImplant, {-150._n, 0._n}, 80._n, 275._n);

        this->addRectangleCenter(Layer::Contact, {-50._n,  35._n}, 30._n, 30._n);
        this->addRectangleCenter(Layer::Contact, { 50._n,  35._n}, 30._n, 30._n);
        this->addRectangleCenter(Layer::Contact, {-50._n, -35._n}, 30._n, 30._n);
        this->addRectangleCenter(Layer::Contact, { 50._n, -35._n}, 30._n, 30._n);
        this->addRectangleCenter(Layer::Contact, {-50._n,  100._n}, 30._n, 30._n);
        this->addRectangleCenter(Layer::Contact, { 50._n,  100._n}, 30._n, 30._n);
        this->addRectangleCenter(Layer::Contact, {-50._n, -100._n}, 30._n, 30._n);
        this->addRectangleCenter(Layer::Contact, { 50._n, -100._n}, 30._n, 30._n);

        this->addRectangleCenter(Layer::Contact, {-150._n,   35._n}, 30._n, 30._n);
        this->addRectangleCenter(Layer::Contact, {-150._n,  -35._n}, 30._n, 30._n);
        this->addRectangleCenter(Layer::Contact, {-150._n,  100._n}, 30._n, 30._n);
        this->addRectangleCenter(Layer::Contact, {-150._n, -100._n}, 30._n, 30._n);
    }

    void Test::routeInstances()
    {
    }
    
    void Test::createLayoutPins()
    {
        debug::unimplement("Test::createLayoutPins");
    }

}