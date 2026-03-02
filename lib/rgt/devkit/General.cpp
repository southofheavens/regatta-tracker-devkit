#include <rgt/devkit/General.h>
#include <rgt/devkit/RGTException.h>

#include <fstream>

namespace RGT::Devkit
{

std::string readLuaScript(const std::string & filename) 
{
    std::ifstream file(filename);
    
    if (not file.is_open()) {
        throw std::runtime_error("Cannot open Lua script: " + filename);
    }
    return std::string
    (
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

} // namespace RGT::Devkit
