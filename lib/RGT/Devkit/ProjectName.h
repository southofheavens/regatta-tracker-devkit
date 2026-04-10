#pragma once

#include <string_view>

namespace RGT::Devkit
{

#ifndef PROJECT_NAME
    #error A PROJECT_NAME variable must been defined
#else

inline const char * getProjectName()
{ return PROJECT_NAME; };

inline const char * getConfigPath()
{ return "/etc/" PROJECT_NAME ".config"; }

#endif

} // namespace RGT::Devkit
