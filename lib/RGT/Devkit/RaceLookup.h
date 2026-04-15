#pragma once

#include <RGT/Devkit/Types.h>
#include <string>

#include <Poco/Redis/PoolableConnectionFactory.h>
#include <Poco/Data/SessionPool.h>

namespace Poco::Data { class Session; } // namespace Poco::Data

namespace RGT::Devkit
{

using RedisClientObjectPool = Poco::ObjectPool<Poco::Redis::Client, Poco::Redis::Client::Ptr>;

/// @brief Проверяет, существует ли гонка с указанным ID
/// @param session psql сессия
/// @param pc cоединение с redis
/// @param raceId ID гонки
/// @return true, если гонка существует и false в противном случае
bool isRaceExists(Poco::Data::Session & session, Poco::Redis::PooledConnection & pc, RaceId raceId);

/// @brief Проверяет, существует ли гонка с указанным ID
/// @param sessionPool пул соединений с psql
/// @param pc cоединение с redis
/// @param raceId ID гонки
/// @return true, если гонка существует и false в противном случае
bool isRaceExists(Poco::Data::SessionPool & sessionPool, Poco::Redis::PooledConnection & pc, RaceId raceId);

/// @brief Проверяет, существует ли гонка с указанным ID
/// @param sessionPool пул соединений с psql
/// @param redisPool пул соединений с redis
/// @param raceId ID гонки
/// @return true, если гонка существует и false в противном случае
bool isRaceExists(Poco::Data::SessionPool & sessionPool, RedisClientObjectPool & redisPool, RaceId raceId);

/// @brief Проверяет, существует ли гонка с указанным ID
/// @param session psql сессия
/// @param redisPool пул соединений с redis
/// @param raceId ID гонки
/// @return true, если гонка существует и false в противном случае
bool isRaceExists(Poco::Data::Session & session, RedisClientObjectPool & redisPool, RaceId raceId);

/* - */

/// @brief Проверяет, входит ли пользователь в список участников гонки (включая тренеров)
/// @param session psql сессия
/// @param pc cоединение с redis
/// @param raceId ID гонки
/// @param userId ID пользователя
/// @return true, если входит и false в противном случае
bool isParticipationExists(Poco::Data::Session & session, Poco::Redis::PooledConnection & pc, 
    RaceId raceId, UserId userId);

/// @brief Проверяет, входит ли пользователь в список участников гонки (включая тренеров)
/// @param sessionPool пул сессий с psql
/// @param pc cоединение с redis
/// @param raceId ID гонки
/// @param userId ID пользователя
/// @return true, если входит и false в противном случае
bool isParticipationExists(Poco::Data::SessionPool & sessionPool, Poco::Redis::PooledConnection & pc, 
    RaceId raceId, UserId userId);

/// @brief Проверяет, входит ли пользователь в список участников гонки (включая тренеров)
/// @param sessionPool пул сессий с psql
/// @param redisPool пул соединений с redis
/// @param raceId ID гонки
/// @param userId ID пользователя
/// @return true, если входит и false в противном случае
bool isParticipationExists(Poco::Data::SessionPool & sessionPool, RedisClientObjectPool & redisPool, 
    RaceId raceId, UserId userId);

/// @brief Проверяет, входит ли пользователь в список участников гонки (включая тренеров)
/// @param session psql сессия
/// @param redisPool пул соединений с redis
/// @param raceId ID гонки
/// @param userId ID пользователя
/// @return true, если входит и false в противном случае
bool isParticipationExists(Poco::Data::Session & session, RedisClientObjectPool & redisPool, 
    RaceId raceId, UserId userId);

/* - */

/// @brief Возвращает статус гонки
/// @param psql сессия
/// @param pc cоединение с redis
/// @param raceId ID гонки
/// @return Статус гонки
/// @throw std::runtime_error если гонка не существует
RaceStatus getRaceStatus(Poco::Data::Session & session, Poco::Redis::PooledConnection & pc, 
    RaceId raceId);

/// @brief Возвращает статус гонки
/// @param sessionPool пул сессий с psql
/// @param pc cоединение с redis
/// @param raceId ID гонки
/// @return Статус гонки
/// @throw std::runtime_error если гонка не существует
RaceStatus getRaceStatus(Poco::Data::SessionPool & sessionPool, Poco::Redis::PooledConnection & pc,
    RaceId raceId);

/// @brief Возвращает статус гонки
/// @param sessionPool пул сессий с psql
/// @param redisPool пул соединений с redis
/// @param raceId ID гонки
/// @return Статус гонки
/// @throw std::runtime_error если гонка не существует
RaceStatus getRaceStatus(Poco::Data::SessionPool & sessionPool, RedisClientObjectPool & redisPool, 
    RaceId raceId);

/// @brief Возвращает статус гонки
/// @param psql сессия
/// @param raceId ID гонки
/// @param redisPool пул соединений с redis
/// @return Статус гонки
/// @throw std::runtime_error если гонка не существует
RaceStatus getRaceStatus(Poco::Data::Session & session, RedisClientObjectPool & redisPool, 
    RaceId raceId);
    
} // namespace RGT::Devkit
