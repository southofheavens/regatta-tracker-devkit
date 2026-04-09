#pragma once

#include <string_view>

namespace RGT::Devkit
{

#ifndef PROJECT_NAME
    #error A PROJECT_NAME variablie must been defined
#else

inline const char * getProjectName()
{ 
    std::string_view projectName = PROJECT_NAME;
    std::string_view devkit = "rgt-devkit";
    if (projectName == devkit) {
        throw std::runtime_error("A PROJECT_NAME variable must been redefined to the name of the current service");
    }
    return PROJECT_NAME;
};

inline const char * getConfigPath()
{ return "/etc/" PROJECT_NAME ".config"; }

#endif

} // namespace RGT::Devkit
