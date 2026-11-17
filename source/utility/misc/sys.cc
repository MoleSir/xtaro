#include "./sys.hh"
#include <cstdlib>

namespace xtaro::util
{

    int execute(std::string_view command)
    {
        int result = std::system(command.data());
        return WEXITSTATUS(result);
    }

    void exit(int status)
    {
        std::exit(status);
    }

}