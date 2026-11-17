#pragma once

#include <string_view>

namespace xtaro::util
{

    int execute(std::string_view command);

    void exit(int status);
    
}