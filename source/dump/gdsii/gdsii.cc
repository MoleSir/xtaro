#include "./gdsii.hh"
#include "parse/gdsii/gdsiilayout.hh"

#include <circuit/base/circuit.hh>
#include <circuit/base/module.hh>
#include <circuit/base/instance.hh>
#include <circuit/component/mos.hh>
#include <circuit/component/resistance.hh>
#include <circuit/component/capacitance.hh>

#include <circuit/geometry/geometry.hh>
#include <circuit/geometry/label.hh>
#include <circuit/geometry/rectangle.hh>
#include <circuit/geometry/path.hh>
#include <circuit/geometry/pin.hh>
#include <circuit/geometry/contact.hh>

#include <format>
#include <global/config/config.hh>
#include <utility/debug/debug.hh>

namespace xtaro::dump 
{

    GDSII::GDSII(circuit::Circuit* circuit, std::filesystem::path outputPath) :
        _outputPath{outputPath},
        _circuit{circuit},
        _visitor{}
    {
    }

    void GDSII::write()
    {
        if (this->_circuit->hasGDSIIFilePath())
        {
            auto& originFilePath = this->_circuit->gdsiiFilePath();
            if (originFilePath == this->_outputPath)
            {
                debug::infof("The module has already been write to gdsii file '{}'", originFilePath.c_str());
            }
            else 
            {
                debug::infof("The module has already been write to gdsii file '{}', now copy it to '{}'", originFilePath.c_str(), this->_outputPath.c_str());
                std::filesystem::copy(originFilePath, this->_outputPath, std::filesystem::copy_options::overwrite_existing);
            }
        }
        
        this->_circuit->accept(this->_visitor);
        auto& layout = this->_visitor.layout(this->_circuit);
        layout->writeGDSII(this->_outputPath);

        this->_circuit->setGDSIIFilePath(std::move(this->_outputPath));
    }

}