#include "file.hh"
#include <format>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace xtaro::util
{
    std::size_t filesize(std::ifstream& file)
    {
        file.seekg(0, std::ios::end);
        std::size_t size{ static_cast<std::size_t>(file.tellg()) };
        file.seekg(0, std::ios::beg);
        return size;
    }

    std::string readFile(std::ifstream& file)
    {
        if (!file.is_open()) 
            throw std::runtime_error {"util::readFile: File does not open."};

        std::size_t size {filesize(file)};
        std::string content {};
        content.resize(size + 1);
        file.read(content.data(), size);
        content.resize(size);
        return content;
    }

    std::string readFile(const std::filesystem::path& filePath)
    {
        auto infile = std::ifstream{filePath};

        if (!infile.is_open()) 
            throw std::runtime_error {std::format(
                "util::readFile: File '{}' does not open.",
                filePath.string()
            )};

        std::size_t size = filesize(infile);
        std::string content {};
        content.resize(size + 1);
        infile.read(content.data(), size);
        content.resize(size);
        return content;

        infile.close();

        return content;
    }

    auto readFileLines(std::ifstream& file) -> std::vector<std::string> 
    {
        if (!file.is_open()) 
            throw std::runtime_error {"util::readFileLines: File does not open."};
    
        auto line = std::string{};
        auto res = std::vector<std::string>{};
        while (std::getline(file, line)) 
            res.emplace_back(std::move(line));
        return res;
    }

    auto readFileLines(const std::filesystem::path& filePath) -> std::vector<std::string>
    {
        auto infile = std::ifstream{filePath};

        if (!infile.is_open()) 
            throw std::runtime_error {std::format(
                "util::readFileLines: File '{}' does not open.",
                filePath.string()
            )};
        
        return readFileLines(infile);
    }

    void writeFile(const std::filesystem::path& filePath, std::string_view content)
    {
        auto outfile = std::ofstream{filePath};
        outfile.write(content.data(), content.size());
        outfile.close();
    }

    bool fileExists(const std::filesystem::path& filePath) 
    {
        return std::filesystem::exists(filePath) && 
               std::filesystem::is_regular_file(filePath);
    }

    bool directoryExists(const std::filesystem::path& dirPath) 
    {
        return std::filesystem::exists(dirPath) && 
               std::filesystem::is_directory(dirPath);
    }

}