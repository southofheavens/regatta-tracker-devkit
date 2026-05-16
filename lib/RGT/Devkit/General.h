#pragma once

#include <string>
#include <optional>

namespace Poco::Util { class AbstractConfiguration; } // namespace Poco::Util

namespace RGT::Devkit
{

// Считывает lua-script из файла с именем filename и возвращает его
std::string readLuaScript(const std::string & filename);

/// @brief Загружает .env из cwd, затем из regatta-tracker-main/.env (для локального запуска).
/// Уже заданные переменные окружения не перезаписываются.
/// @throw std::runtime_error при ошибке разбора
void readDotEnv();

std::optional<std::string> getEnv(const std::string & envVarName);

std::optional<std::string> getEnvOrCfg(const std::string & envVarName, const std::string & cfgVarName,
    const Poco::Util::AbstractConfiguration & cfg);

} // namespace RGT::Devkit
