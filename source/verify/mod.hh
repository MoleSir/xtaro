#pragma once

#include <filesystem>
#include <optional>

namespace xtaro::circuit {
    class Circuit;
}

namespace xtaro::verify 
{

    /*
        Run DRC check on a cell which is implemented in gds_name.
        
        @return: How many drc errors
    */
    auto runDrc(
        std::string_view exe, circuit::Circuit* cricuit, 
        bool extract, bool finalVerification, std::optional<std::filesystem::path> scriptFolder = std::nullopt) -> int;

    /*
        Run LVS check on a given top-level name which is
        implemented in gds_name and sp_name. Final verification will
        ensure that there are no remaining virtual conections.

        @return: How many drc errors
    */
    auto runLvs(
        std::string_view exe, circuit::Circuit* cricuit, 
        bool finalVerification, std::optional<std::filesystem::path> scriptFolder = std::nullopt) -> int;

    /*
        Write a klayout script to perform DRC and optionally extraction.

        @return: script file path
    */
    auto writeDrcScript(
        std::string_view exe, circuit::Circuit* cricuit, 
        bool extract, bool finalVerification, std::optional<std::filesystem::path> scriptFolder = std::nullopt) -> std::filesystem::path;

    /*
        Write a netgen script to perform LVS.

        @return: script file path
    */
    auto writeLvsScript(
        std::string_view exe, circuit::Circuit* cricuit, 
        bool finalVerification, std::optional<std::filesystem::path> scriptFolder = std::nullopt) -> std::filesystem::path;

}