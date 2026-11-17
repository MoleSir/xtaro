#include "./spice.hh"

#include <utility/debug/debug.hh>
#include <global/config/config.hh>
#include <utility/misc/file.hh>

namespace xtaro::dump
{

    Spice::Spice(circuit::Circuit* circuit, std::filesystem::path outputPath) :
        _outputPath{std::move(outputPath)},
        _circuit{circuit},
        _visitor{_outputPath}
    {
    }

    auto Spice::write() -> void 
    {
        if (this->_circuit->hasSpiceFilePath())
        {
            auto& originFilePath = this->_circuit->spiceFilePath();
            if (originFilePath == this->_outputPath)
            {
                debug::infof("The module has already been write to spice file '{}'", originFilePath.c_str());
            }
            else 
            {
                debug::infof("The module has already been write to spice file '{}', now copy it to '{}'", originFilePath.c_str(), this->_outputPath.c_str());
                std::filesystem::copy(originFilePath, this->_outputPath, std::filesystem::copy_options::overwrite_existing);
            }
            return;
        }
        
        this->_circuit->accept(this->_visitor);
        this->_circuit->setSpiceFilePath(std::move(this->_outputPath));
    }

}