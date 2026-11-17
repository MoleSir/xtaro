#include "./spicer.hh"
#include "./ngspice.hh"
#include "./spectre.hh"
#include "utility/debug/debug.hh"
#include <memory>

namespace xtaro::characterize {

    auto Spicer::from(std::string_view spicer) -> std::unique_ptr<Spicer>
    {
        if (spicer == "ngspice") {
            return std::make_unique<NGSpice>();
        }
        if (spicer == "spectre") {
            return std::make_unique<Spectre>();
        }

        debug::exceptionf("Unspport spice simulator '{}'", spicer);
    }

}