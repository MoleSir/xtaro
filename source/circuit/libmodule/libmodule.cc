#include "libmodule.hh"
#include "circuit/base/port.hh"
#include "circuit/geometry/coord.hh"
#include "number/number.hh"
#include "utility/misc/file.hh"

#include <circuit/mod.hh>

#include <global/config/config.hh>
#include <global/tech/tech.hh>

#include <string>
#include <string_view>
#include <tuple>
#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>
#include <vector>

namespace xtaro::circuit
{

    auto LibModule::Arguments::toString() const -> std::string
    {
        return std::format("{}", this->moduleName);
    }

    auto LibModule::Arguments::toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit>
    {
        return std::make_unique<LibModule>(circuitName, *this);
    }

    LibModule::LibModule(std::string_view name, const LibModule::Arguments& arguments) :
        Module{name},
        _moduleName{arguments.moduleName},
        _property{arguments.property},
        _parent{arguments.parent}
    {
        // Reset this name
        if (this->_property.has_value()) 
            this->_name = this->_property.value()->name;
        else 
            this->_name = circuit::string(arguments.moduleName);

        // Build this
        // Spice must!
        auto spiceFilePath = tech->spiceFilePath(std::string{this->_name});
        this->loadNetlistInfoFromFile(spiceFilePath);
        // Layout?
        if (config->buildBackend)
        {
            auto gdsiiFilePath = tech->gdsiiFilePath(std::string{std::string{this->_name}});
            this->loadLayoutInfoFromFile(gdsiiFilePath);
        }

        // Add instance!
        auto nets = this->connectNets();
        auto inst = this->_parent->addInstance("inner", this, nets);

        // Copy pins if layout
        if (config->buildBackend) 
        {
            for (auto i = 0; i < this->_ports.size(); ++i)
            {
                auto parentPort = this->_parent->ports().at(i);
                auto thisPort = this->ports().at(i);
                this->_parent->copyInstancePins(inst, thisPort.name(), parentPort.name());
            } 
            this->_parent->setWidth(this->width());
            this->_parent->setHeight(this->height());
        }
    }

    void LibModule::loadNetlistInfoFromFile(const std::filesystem::path& path) 
    {
        auto rawlines = util::readFileLines(path);
        // Combined '+'
        auto lines = std::vector<std::string>{};
        int row = 0;
        while (row < rawlines.size()) 
        {
            // get a line
            auto line = std::string{};
            line.append(rawlines.at(row));
            row += 1;

            // Is concat?
            while (row < rawlines.size() && rawlines.at(row).starts_with('+')) {
                line.append(rawlines.at(row).substr(1));
                row += 1;
            }

            lines.emplace_back(std::move(line));
        }

        for (const auto& line : lines) 
        {
            if (line.empty()) continue;
            auto trimLine = util::stringTrim(line);
            // auto lowLine = util::stringToLowercase(trimLine);
            // Get line with .subckt
            if (trimLine.starts_with(".subckt") || trimLine.starts_with(".SUBCKT"))
            {
                // .subckt and_2_x0 A1 A2 Z VDD VSS
                auto tokens = util::splitAll(' ', trimLine);
                for (auto i = 2; i < tokens.size(); ++i) 
                {
                    this->addPort(circuit::string(std::string{tokens.at(i)}), PortType::InOut);
                }

                debug::checkf(this->_name == tokens[1], "Unmath name in libmodule: '{}' with '{}'", this->_name, tokens[1]);
                this->setSpiceFilePath(path);
                return;
            }
        }

        // No exit `.subckt` in spice file!
        debug::exceptionf("Invalid spice file format in '{}'", path.string());
    }

    void LibModule::loadLayoutInfoFromFile(const std::filesystem::path& gdsiiFilePath) 
    {
        auto layout = parse::GDSIILayout::loadFromFile(gdsiiFilePath);
        auto [w, l] = layout->measureLayerSize(tech->layerMask(Layer::PRBoundary));
        this->_width.emplace(w, Unit::Micro);
        this->_height.emplace(l, Unit::Micro);

        for (auto& port : this->_ports)
        {
            auto pinInfo = layout->pinInfo(std::string{port.name()});

            for (auto& [layerMask, rect] : pinInfo) 
            {
                auto& ld = rect.leftdown;
                auto& ru = rect.rightup;

                auto offset = Coord{Number{ld.x, Unit::Micro}, Number{ld.y, Unit::Micro}};
                auto pinWidth  = Number{ru.x - ld.x, Unit::Micro};
                auto pinHeight = Number{ru.y - ld.y, Unit::Micro};

                auto layer = tech->layerFromMask(layerMask);
                this->addPin(layer, port.name(), offset, pinWidth, pinHeight);
            }
        }

        this->setGDSIIFilePath(gdsiiFilePath);

        // debug::debugf("Load gdsii for '{}'", this->_name);
        // debug::debugf(" - width: {}", *this->_width);
        // debug::debugf(" - height: {}", *this->_height);
        // debug::debug(" - pins:");
        // for (auto& [name, pin] : this->_pins) {
        //     debug::debugf("   - {}:", name);
        //     for (auto& p : pin) {
        //         debug::debugf("     - {}, [{}, {}]", p.layer(), p.leftdown(), p.rightup());
        //     }
        // }
    }

    auto LibModule::connectNets() const -> std::vector<std::string_view> 
    {
        const auto& parentPorts = this->_parent->ports();
        auto nets = std::vector<std::string_view>{};
        if (!this->_property.has_value()) 
        {
            // Use default connect: wrap module net!
            for (const auto& port : parentPorts) 
                nets.emplace_back(port.name());
        }
        else 
        {
            for (const auto& name : this->_property.value()->portsOrder) {
                nets.emplace_back(circuit::string(name));
            }
        }

        return nets;
    }

}