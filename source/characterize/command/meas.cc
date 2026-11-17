#include "meas.hh"
#include <global/number/number.hh>

#include <utility/debug/debug.hh>
#include <utility/misc/string.hh>
#include <optional>
#include <regex>
#include <cmath>

namespace xtaro::characterize
{

    Measurement::Measurement(std::string name) : 
        _name{std::move(name)}, _result{std::nullopt} 
    {
    }

    bool Measurement::getMeasureResult(const std::string& context)
    {
        std::regex nameResultRegex{
            std::format("{}\\s*=\\s*-?\\d+.?\\d*[e]?[-+]?[0-9]+", this->_name)
        };

        // Find 'name = result'
        std::smatch nameResultMatch;
        bool success = 
            std::regex_search(context, nameResultMatch, nameResultRegex);
        if (success == false)
        {
            debug::warningf("Get '{}'.meas result failed!", this->_name);
            return false;
        }
        
        // Find 'result'
        std::string nameValue{nameResultMatch[0].str()};
        std::size_t pos {nameValue.find('=')};
        while (nameValue[++pos] != ' ') {}

        // Get value
        std::string value {nameValue.substr(pos)};
        this->_result.emplace(
            Number::origin(std::stod(value))
        );

        return true;
    }
    
}