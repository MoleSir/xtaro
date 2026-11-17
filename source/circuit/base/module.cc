#include "./module.hh"
#include "./circuit.hh"
#include "./instance.hh"
#include "./net.hh"
#include "./port.hh"
#include "./visitor.hh"
#include "circuit/geometry/coord.hh"
#include "circuit/libmodule/libmodule.hh"

#include <memory>
#include <utility/debug/debug.hh>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <circuit/mod.hh>

#include <string_view>
#include <utility/misc/file.hh>
#include <utility/misc/string.hh>

#include <string>
#include <vector>
#include <algorithm>

namespace xtaro::circuit {

    Module::Module(std::string_view name) :
        Circuit{name},
        _circuits{},
        _instances{},
        _nets{},
        _connectedNets{}
    {
    }

    Module::~Module() noexcept
    {        
    }

    void Module::addPortNet(std::string_view name, PortType type)
    {
        this->addPort(name, type);
        this->addNet(name);
    }

    void Module::addPortNets(const std::vector<std::string_view>& portsName, PortType portType)
    {
        for (const std::string_view& portName : portsName)
            this->addPortNet(portName, portType);
    }

    auto Module::addInstance(std::string_view instanceName, Circuit* circuit, const std::vector<std::string_view>& netsName) -> Instance*
    {
        // Create instance
        auto& instance = this->_instances.emplace_back(std::make_unique<Instance>(instanceName, circuit));
        
        // Connect
        if (instance->ports().size() != netsName.size())
        {
            std::string errorMsg {
                std::format("In circuit '{}', when connect with instance '{}', Port size '{}' != Net size '{}'", 
                             this->_name,
                             instance->name(),
                             instance->ports().size(), 
                             netsName.size())
            };

            debug::exceptionIn("Connect instance", errorMsg);
        }

        std::vector<Net*> nets = this->addNets(netsName);
        instance->connectNets(nets);
        
        return instance.get();
    }

    auto Module::addNet(std::string_view netName) -> Net*
    {
        auto res = this->_nets.emplace(netName, netName);
        return &(res.first->second);
    }

    auto Module::addNets(const std::vector<std::string_view>& netsName) -> std::vector<Net*>
    {
        auto nets = std::vector<Net*>{};
        std::transform(
            netsName.cbegin(), netsName.cend(), std::back_inserter(nets),
            [this] (const std::string_view& netName) { return this->addNet(netName); }
        );
        return nets;
    }

    void Module::connectNets(std::string_view net1, std::string_view net2)
    {
        auto net1Obj = this->addNet(net1);
        auto net2Obj = this->addNet(net2);
        this->_connectedNets.emplace_back(net1Obj, net2Obj);
    }

    auto Module::addCircuit(const Circuit::Arguments& arguments) -> Circuit*
    {
        auto cir = circuit::circuit(arguments);
        this->_circuits.emplace(cir);
        return cir;
    }

    void Module::createNetlist()
    {
        this->createCircuits();
        this->createInstances();
    }

    auto Module::createLayout() -> void
    {
        this->calculateLayoutConstants();
        this->placeInstances();
        this->routeInstances();
        this->createLayoutPins();
        this->layoutMiscellaneous();
    }

    auto Module::getBlockages(Layer layer) const -> std::vector<Blockage>
    {
        auto blockages = this->Circuit::getBlockages(layer);
        for (const auto& inst : this->_instances) 
        {
            auto instBlockages = inst->getBlockages(layer);
            blockages.insert(blockages.end(), instBlockages.begin(), instBlockages.end());
        }
        return blockages;
    }

    auto Module::containsInstance(Instance* inst) -> bool
    {  
        for (auto& i : this->_instances) {
            if (i.get() == inst) 
                return true;
        }
        return false;
    }

    auto Module::copyInstancePin(Instance* inst, std::string_view pinName, std::optional<std::string_view> newName) -> void
    {
        if (!this->containsInstance(inst))
            debug::exceptionf("'{}' is not an instance of circuit '{}'", inst->name(), this->name());

        auto pin = inst->getPin(pinName);
        auto name = newName.has_value() ? *newName : pinName;
        this->addPin(pin.layer(), name, pin.leftdown(), pin.width(), pin.height());
    }

    auto Module::copyInstancePins(Instance* inst, std::string_view pinName, std::optional<std::string_view> newName) -> void
    {
        if (!this->containsInstance(inst))
            debug::exceptionf("'{}' is not an instance of circuit '{}'", inst->name(), this->name());

        auto pins = inst->getPins(pinName);
        auto name = newName.has_value() ? *newName : pinName;
        for (auto& pin : pins) {
            this->addPin(pin.layer(), name, pin.leftdown(), pin.width(), pin.height());
        }
    }

    auto Module::accept(CircuitVisitor& visitor) -> void
    {
        return visitor.visitModule(this);
    }

    void Module::placeInstArrayInColumn(std::span<Instance*> insts, const Number& xOff, const Number& startY)
    {
        if (insts.size() == 0) 
            return;

        auto& instHeight = insts.front()->circuit()->height();

        for (auto i = 0; i < insts.size(); ++i) 
        {
            auto inst = insts[i];
            auto yOff = ((i % 2 == 0) ? i * instHeight : (i+1) * instHeight) + startY;
            auto mirror = (i % 2 == 0) ? MirrorType::None : MirrorType::MirrorY;
            inst->place(Coord{xOff, yOff}, mirror);
        }
    }

    void Module::placeInstArrayInRow(std::span<Instance*> insts, const Number& yOff, const Number& startX) 
    {
        if (insts.size() == 0) 
            return;

        auto xOff = startX;

        for (auto i = 0; i < insts.size(); ++i) 
        {
            auto inst = insts[i];
            inst->place(Coord{xOff, yOff});
            xOff += inst->circuit()->width();
        }
    }

    auto Module::tryLoadFromLibrary() -> void 
    {
        auto libModuleName = (this->_name.starts_with("wrap_")) ? (this->_name.substr(5)) : (this->_name);
        auto property = tech->moduleProperty(libModuleName);
        auto circuit = property.has_value() ? property.value()->name : libModuleName;

        try {
            // Try Load lib module!
            tech->spiceFilePath(std::string{circuit});
            if (config->buildBackend) 
                tech->gdsiiFilePath(std::string{std::string{circuit}});
        } catch (const std::exception& _) {
            debug::warningf("No exit lib module '{}', so generate it", circuit);
            this->createSelf();
            return;
        }

        this->loadFromLibrary();
    }

    auto Module::loadFromLibrary() -> void 
    {
        // remove 'wrap_' header!
        auto libModuleName = (this->_name.starts_with("wrap_")) ? (this->_name.substr(5)) : (this->_name);
        auto property = tech->moduleProperty(libModuleName);
        this->createPorts();
        this->addCircuit(LibModule::Arguments{
            std::string{libModuleName}, 
            property,
            this
        });
    }

}