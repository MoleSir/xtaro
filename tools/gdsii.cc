#include "utility/debug/debug.hh"
#include <parse/gdsii/gdsiilayout.hh>

using namespace xtaro;

int main(int argc, char* argv[]) 
{
    debug::setDebugLevel(debug::DebugLevel::Debug);
    auto l = parse::GDSIILayout::loadFromFile(argv[1]);
    l->writeTxt("dump.txt");
    l->writeGDSII("dump.gds");
}