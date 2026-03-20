#ifndef __GENERAL_H__
#define __GENERAL_H__

#include <string>

namespace RGT::Devkit
{

// Считывает lua-script из файла с именем filename и возвращает его
std::string readLuaScript(const std::string & filename);

/// @brief Считывает и устанавливает переменные окружения, находящиеся в файле .env, который расположен
/// на одном уровне с meson.build. Если переменная окружения уже задана, перезаписи не будет.
/// @throw std::runtime_error при ошибке (отсутствует .env, переменные окружения некорректно заданы и т.д.)
void readDotEnv();

} // namespace RGT::Devkit

#endif // __GENERAL_H__

