#pragma once

#include "./characterizer/delay.hh"
#include "characterize/characterizer/setuphold.hh"

#include <circuit/module/sram.hh>
#include <global/env/pvt.hh>
#include <global/env/slewload.hh>
#include <global/env/slews.hh>

namespace xtaro::characterize
{

    /*
        Execute function test to `sram` with given `pvt` and `slew` and `load`
    */
    bool functionTest(std::string_view policy, circuit::SRAM* sram, PVT pvt, const SlewLoad& slewload);

    /*
        Execute delay characterize to `sram` with given `pvt` and `slew` and `load`
    */
    auto analyzeDelay(circuit::SRAM* sram, PVT pvt, const SlewLoad& slewload) -> DelayResult;
    auto analyzeDelays(circuit::SRAM* sram, PVT pvt, std::span<Number> slews, std::span<Number> loads) -> std::unordered_map<SlewLoad, DelayResult>;
    auto analyzeDelays(circuit::SRAM* sram, PVT pvt, std::span<SlewLoad> slewloads) -> std::unordered_map<SlewLoad, DelayResult>;
    void displayDelay(circuit::SRAM* sram, PVT pvt, const SlewLoad& slewload);

    /*
        Execute setup hold characterize to cur technology with given `pvt` and `slews`
    */
    auto analyzeSetuphold(PVT pvt, const Slews& slewss) -> SetupHoldResult;
    auto analyzeSetupholds(PVT pvt, std::span<Number> relatedSlews, std::span<Number> constrainedSlews) -> std::unordered_map<Slews, SetupHoldResult>;
    auto analyzeSetupholds(PVT pvt, std::span<Slews> slewss) -> std::unordered_map<Slews, SetupHoldResult>;
    void disaplySetuphold(PVT pvt, const Slews& slewss);

}