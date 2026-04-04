#pragma once

#include <Poco/Data/SessionPool.h>

namespace Poco::Util { class AbstractConfiguration; } // namespace Poco::Util

namespace RGT::Devkit::Connections
{

/// @brief Создаёт и инициализирует пул соединений с PostgreSQL
/// @return Указатель на пул сессий
/// @throw std::runtime_error при ошибке подключения
std::unique_ptr<Poco::Data::SessionPool> connectToPsql
(
    const std::string & host,
    const std::string & port,
    const std::string & dbname,
    const std::string & user, 
    const std::string & password,
    const uint8_t       minSessions,
    const uint8_t       maxSessions
); 

/// @brief Создает и инициализирует пул соединений с PostgreSQL, используя
/// параметры из конфига и переменных окружения
/// @return Указатель на пул сессий
/// @throw std::runtime_error при ошибке подключения
std::unique_ptr<Poco::Data::SessionPool> connectToPsql
(
    const Poco::Util::AbstractConfiguration & cfg
);

} // RGT::Devkit::Connections
