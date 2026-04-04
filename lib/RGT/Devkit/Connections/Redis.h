#pragma once

#include <Poco/Redis/PoolableConnectionFactory.h>

namespace Poco::Util { class AbstractConfiguration; } // namespace Poco::Util

namespace RGT::Devkit
{

using RedisClientObjectPool = Poco::ObjectPool<Poco::Redis::Client, Poco::Redis::Client::Ptr, 
    Poco::PoolableObjectFactory<Poco::Redis::Client, Poco::Redis::Client::Ptr>>;

namespace Connections
{

/// @brief Создаёт и инициализирует пул соединений с Redis
/// @return Указатель на пул сессий
/// @throw std::runtime_error при ошибке подключения
std::unique_ptr<RedisClientObjectPool> connectToRedis
(
    const std::string & host, 
    const std::string & port,
    const uint8_t       minSessions,
    const uint8_t       maxSessions
);

/// @brief Создаёт и инициализирует пул соединений с Redis, используя
/// параметры из конфига и переменных окружения
/// @return Указатель на пул сессий
/// @throw std::runtime_error при ошибке подключения
std::unique_ptr<RedisClientObjectPool> connectToRedis
(
    const Poco::Util::AbstractConfiguration & cfg
);

} // namespace RGT::Devkit::Connections

} // namespace RGT::Devkit
