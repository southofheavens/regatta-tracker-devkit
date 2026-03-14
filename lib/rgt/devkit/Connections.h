#ifndef __CONNECTIONS_H__
#define __CONNECTIONS_H__

#include <memory>

#include <Poco/Data/SessionPool.h>
#include <Poco/ObjectPool.h>
#include <Poco/Redis/Client.h>

#include <aws/core/auth/signer/AWSAuthV4Signer.h>

namespace Aws 
{ 

namespace S3
{
    class S3Client; 
} // namespace Aws::S3

namespace Http
{
    enum class Scheme;
} // namespace Aws::Http

} // namespace Aws

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

/// @brief Создаёт и инициализирует соединение с S3 хранилищем
/// @return Указатель на S3Client
/// @throw Poco::Exception при ошибке подключения
std::unique_ptr<Aws::S3::S3Client> connectToS3
(
    const std::string & accessKeyId,
    const std::string & secretKey,
    const std::string & endpointOverride,
    const std::string & region, 
    const Aws::Http::Scheme & scheme,
    bool verifySsl,
    bool useVirtualAddressing,
    Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy signingPolicy = 
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never
);

} // namespace RGT::Devkit

#endif // __CONNECTIONS_H__
