#pragma once

#include "./circuit.hh"
#include "./instance.hh"
#include "./net.hh"
#include "./port.hh"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <fstream>
#include <string_view>

namespace xtaro::circuit
{
    
    /*
        A base abstract class for all circuit. 
        Each 'Circuit' object stands for a '.SUBCKT' in spice file.

        A Module need to impl 
        - `createPorts`

        - For netlist
            - createCircuits
            - createInstances
        
        - For layout
            - calculateLayoutConstants
            - placeInstances
            - routeInstances
            - layoutMiscellaneous(if you need)
        
        and then chosie one way to build module:
        - createCircuit: it will invoke `createPorts`, `createCircuits` and `createInstances`(if ness)
        - loadFromLibrary: invoke `createPorts`, and then load the the given module in tech library, load spice and gdsii(if ness)
        - tryLoadFromLibrary: try `loadFromLibrary` first, if failed, invoke `createCircuit`

    */
    class Module : public Circuit
    {
    public:
        /*
            Circuit construction.
            - name: This circuit's name. In spice, it is the name of a '.SUBCKT';
            - type: The spice device type of this circuit. e.g, MOS, Diode, Capacitance, Resistance, SUBCKT;
            - spicefile: The spice description file of this circuit if it exits. (Default value: "").
        */
        Module(std::string_view name);
        virtual ~Module() noexcept override;

    public:
        virtual auto accept(CircuitVisitor& visitor) -> void override;

    public:
        /*
            Add an port to this circuit.
            - portName: the port's name;
            - portType: the port's type.
        */
        void addPortNet(std::string_view portName, PortType portType);

        /*
            Add ports to this circuit.
            - portName: the ports' name;
            - portType: the ports' type.
        */
        void addPortNets(const std::vector<std::string_view>& portsName, PortType portType);

        /*
            Add an Instance object to this circuit. And connet it with nets.
        */
        auto addInstance(std::string_view instanceName, Circuit* circuit, const std::vector<std::string_view>& nets) -> Instance*;
        
        /*
            Add a Cricuit object to this circuit.
        */
        auto addCircuit(const Circuit::Arguments& arguments) -> Circuit*;

        /*
            Connect two net with each other
        */
        void connectNets(std::string_view net1, std::string_view net2);
        
        /*
            Create a new circuit:
            - Add ports
            - Create Sub Circuits
            - Create and connect instance
        */
        virtual auto createNetlist() -> void override;
        virtual auto createLayout()  -> void override;

        auto getBlockages(Layer layer) const -> std::vector<Blockage>;

    public:
        auto containsInstance(Instance* inst) -> bool;

        auto copyInstancePin(Instance* inst, std::string_view pinName, std::optional<std::string_view> newName = std::nullopt) -> void;
        auto copyInstancePins(Instance* inst, std::string_view pinName, std::optional<std::string_view> newName = std::nullopt) -> void;

    private:
        /*
            Add a Net object whose name is 'netName'
        */
       auto addNet(std::string_view netName) -> Net*;

        /*
            Add Net objects in this circuit whose name in 'netsName'. 
            If netname is already been created, just return it, or create a new Net object with net name.
            And finally, return Net objects as a vector.
            - netsName: the names vector of Net objects.
        */
        auto addNets(const std::vector<std::string_view>& netsName) -> std::vector<Net*>;

        /*
            The help method for 'writeSpice'. Use recursion
        */
        void doWriteSpice(std::ofstream& file, std::set<const Circuit*>& visited) const;

    protected:
        // virtual auto createPorts() -> void = 0;

        virtual void createCircuits() = 0;
        virtual void createInstances() = 0;

        virtual void calculateLayoutConstants() = 0;
        virtual void placeInstances() = 0;
        virtual void routeInstances() = 0;
        virtual void createLayoutPins() {}
        virtual void layoutMiscellaneous() 
        { this->addPRBoundary(); };

    protected:
        auto loadFromLibrary() -> void;
        auto tryLoadFromLibrary() -> void;

    protected:
        static void placeInstArrayInColumn(std::span<Instance*> insts, const Number& xOff, const Number& startY);
        static void placeInstArrayInRow(std::span<Instance*> insts, const Number& yOff, const Number& startX);

    public:
        virtual std::string argumentsList() const 
        { return ""; }

    public:
        auto subCircuits() const -> const std::unordered_set<Circuit*>&
        { return this->_circuits; }

        auto instances() const -> const std::vector<std::unique_ptr<Instance>>& 
        { return this->_instances; }

        auto connectedNets() const -> const std::vector<std::pair<Net*, Net*>>&
        { return this->_connectedNets; }

    protected:
        std::unordered_set<Circuit*> _circuits;
        std::vector<std::unique_ptr<Instance>> _instances;
        std::unordered_map<std::string_view, Net> _nets;
        std::vector<std::pair<Net*, Net*>> _connectedNets;
    };

}