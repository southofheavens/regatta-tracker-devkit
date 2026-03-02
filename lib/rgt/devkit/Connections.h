#ifndef __CONNECTIONS_H__
#define __CONNECTIONS_H__

#include <memory>

#include <Poco/Data/SessionPool.h>
#include <Poco/ObjectPool.h>
#include <Poco/Redis/Client.h>

namespace RGT::Devkit
{

/// @brief Создаёт и инициализирует пул соединений с PostgreSQL
/// @return Указатель на пул сессий
/// @throw Poco::Exception при ошибке подключения
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

using RedisClientObjectPool = Poco::ObjectPool<Poco::Redis::Client, Poco::Redis::Client::Ptr>;

/// @brief Создаёт и инициализирует пул соединений с Redis
/// @return Указатель на пул сессий
/// @throw Poco::Exception при ошибке подключения
std::unique_ptr<RedisClientObjectPool> connectToRedis
(
    const std::string & host, 
    const std::string & port,
    const uint8_t       minSessions,
    const uint8_t       maxSessions
);

} // namespace RGT::Devkit

#endif // __CONNECTIONS_TO_STORAGE_H__
