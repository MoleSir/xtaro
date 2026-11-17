#include "compiler.hh"
#include <utility/debug/debug.hh>
#include <exception>

int main(int argc, const char* argv[]) 
{
    try 
    {
        xtaro::compiler::main(argc, argv);
    }
    catch (const std::exception& err) 
    {
        xtaro::debug::error(err.what());
    }
    return 0;
}