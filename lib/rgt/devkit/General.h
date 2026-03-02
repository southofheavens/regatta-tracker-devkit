#ifndef __GENERAL_H__
#define __GENERAL_H__

#include <string>

namespace RGT::Devkit
{

// Считывает lua-script из файла с именем filename и возвращает его
std::string readLuaScript(const std::string & filename);

} // namespace RGT::Devkit

#endif // __GENERAL_H__

