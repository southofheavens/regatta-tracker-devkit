#pragma once

#include <string>
#include <optional>

namespace Poco::Util { class AbstractConfiguration; } // namespace Poco::Util

namespace RGT::Devkit
{

// Считывает lua-script из файла с именем filename и возвращает его
std::string readLuaScript(const std::string & filename);

/// @brief Считывает и устанавливает переменные окружения, находящиеся в файле .env, который расположен
/// на одном уровне с meson.build. Если переменная окружения уже задана, перезаписи не будет.
/// @throw std::runtime_error при ошибке (отсутствует .env, переменные окружения некорректно заданы и т.д.)
void readDotEnv();

std::optional<std::string> getEnv(const std::string & envVarName);

std::optional<std::string> getEnvOrCfg(const std::string & envVarName, const std::string & cfgVarName,
    const Poco::Util::AbstractConfiguration & cfg);

} // namespace RGT::Devkit
