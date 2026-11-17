#include "./mod.hh"
#include "characterize/function/function.hh"
#include <circuit/module/sram.hh>
#include <circuit/mod.hh>

#include <characterize/function/function.hh>
#include <characterize/characterizer/delay.hh>
#include <characterize/characterizer/delay.hh>

#include <characterize/characterizer/setuphold.hh>
#include <characterize/simulator/circuitsimulator.hh>

#include <exception>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <utility/debug/debug.hh>
#include <utility/console/console.hh>
#include <utility/misc/sys.hh>
#include <utility/misc/file.hh>
#include <utility/misc/string.hh>

namespace xtaro::characterize
{

    //=============================================================================================//
    //                    Function Test                                                            //
    //=============================================================================================//

    bool functionTest(std::string_view policy, circuit::SRAM* sram, PVT pvt, const SlewLoad& slewload)
    try
    {
        debug::infof(
            "Functional test in {}, {}, using '{}' policy", 
            pvt.toString(), slewload.toString(), policy
        );
        auto tester = characterize::FunctionTester::create(policy, sram, std::move(pvt), slewload);

        return tester->test();
    }
    catch (const std::exception& err)
    {
        debug::exceptionIn("Function test", err.what());
        return false;
    }


    //=============================================================================================//
    //                    Delay                                                                    //
    //=============================================================================================//

    auto analyzeDelay(circuit::SRAM* sram, PVT pvt, const SlewLoad& slewload) -> DelayResult
    try
    {
        debug::infof(
            "Delay character in {}, {}", 
            pvt.toString(), slewload.toString()
        );
        auto characterizer = DelayCharacterizer{sram, std::move(pvt), slewload};
        return characterizer.analyze();
    }
    catch (const std::exception& err)
    {
        debug::exceptionIn("Analyze delay", err.what());
    }

    auto analyzeDelays(circuit::SRAM* sram, PVT pvt, std::span<Number> slews, std::span<Number> loads) -> std::unordered_map<SlewLoad, DelayResult>
    {
        std::vector<SlewLoad> slewloads {};
        for (Number slew : slews)
            for (Number load : loads)
                slewloads.emplace_back(slew, load);
        return analyzeDelays(sram, std::move(pvt), slewloads);
    }

    auto analyzeDelays(circuit::SRAM* sram, PVT pvt, std::span<SlewLoad> slewloads) -> std::unordered_map<SlewLoad, DelayResult>
    try
    {
        std::unordered_map<SlewLoad, DelayResult> results {};
        for (const SlewLoad& slewload : slewloads)
        {
            DelayCharacterizer characterizer {sram, pvt, slewload};
            debug::debugf("Analyze delay info in {}, {}", pvt.toString(), slewload.toString());
            results.emplace(slewload, characterizer.analyze());
        }
        return results;
    }
    catch (const std::exception& err)
    {
        debug::exceptionIn("Analyze delays", err.what());
    }

    void displayDelay(circuit::SRAM* sram, PVT pvt, const SlewLoad& slewload)
    {
        auto result = analyzeDelay(sram, std::move(pvt), slewload);
        debug::infof("dalay_hl {}", result.delayHL);
        debug::infof("dalay_ll {}", result.delayLH);
        debug::infof("slew_hl {}", result.slewHL);
        debug::infof("slew_ll {}", result.slewLH);
    }

    //=============================================================================================//
    //                    Setuphold                                                                //
    //=============================================================================================//

    auto analyzeSetuphold(PVT pvt, const Slews& slewss) -> SetupHoldResult
    try
    {
        debug::infof(
            "Setup Hold character in {}, {}", 
            pvt.toString(), slewss.toString()
        );
        characterize::SetupHoldCharacterizer characterizer {std::move(pvt), slewss};
        return characterizer.analyze();
    }
    catch (const std::exception& err)
    {
        debug::exceptionIn("Analyze setuphold", err.what());
    }

    auto analyzeSetupholds(PVT pvt, std::span<Number> relatedSlews, std::span<Number> constrainedSlews) -> std::unordered_map<Slews, SetupHoldResult>
    {
        std::vector<Slews> slewss {};
        for (const Number relatedSlew : relatedSlews)
            for (const Number constrainedSlew : constrainedSlews)
                slewss.emplace_back(relatedSlew, constrainedSlew);
        return analyzeSetupholds(std::move(pvt), slewss);
    }

    auto analyzeSetupholds(PVT pvt, std::span<Slews> slewss) -> std::unordered_map<Slews, SetupHoldResult>
    try
    {
        std::unordered_map<Slews, SetupHoldResult> result {};
        for (const Slews& slews : slewss)
        {
            SetupHoldCharacterizer characterizer {pvt, slews};
            debug::debugf("Analyze setup hold info in {}, {}", pvt.toString(), slews.toString());
            result.emplace(slews, characterizer.analyze());
        }
        return result;
    }
    catch (const std::exception& err)
    {
        debug::exceptionIn("Analyze setupholds", err.what());
    }

    void disaplySetuphold(PVT pvt, const Slews& slewss)
    {
        auto result = analyzeSetuphold(std::move(pvt), slewss);
        debug::infof("setup_hl {}", result.setupHL);
        debug::infof("setup_hl {}", result.setupLH);
        debug::infof("hold_hl {}", result.holdHL);
        debug::infof("hold_hl {}", result.holdHL);
    }

}