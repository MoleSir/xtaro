#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <vector>

namespace xtaro::util
{
    auto filesize(std::ifstream& file) -> std::size_t;

    auto readFile(std::ifstream& file) -> std::string;
    auto readFile(const std::filesystem::path& filePath) -> std::string;

    auto readFileLines(std::ifstream& file) -> std::vector<std::string>;
    auto readFileLines(const std::filesystem::path& filePath) -> std::vector<std::string>;
    
    void writeFile(const std::filesystem::path& filePath, std::string_view content);
    
    bool fileExists(const std::filesystem::path& filePath);
    bool directoryExists(const std::filesystem::path& dirPath);

}