#include <rgt/devkit/Connections.h>

#include <Poco/Redis/PoolableConnectionFactory.h>

namespace RGT::Devkit
{

std::unique_ptr<Poco::Data::SessionPool> connectToPsql
(
    const std::string & host,
    const std::string & port,
    const std::string & dbname,
    const std::string & user, 
    const std::string & password,
    const uint8_t       minSessions,
    const uint8_t       maxSessions
)
{
    std::string connectionString = std::format
    (
        "host={0} port={1} dbname={2} user={3} password={4}", 
        host, port, dbname, user, password
    );

    std::unique_ptr<Poco::Data::SessionPool> sessionPool = std::make_unique<Poco::Data::SessionPool>("PostgreSQL", 
        connectionString, minSessions, maxSessions);

    try 
    {
        // Проверяем подключение к БД
        sessionPool->get() << "SELECT 1", Poco::Data::Keywords::now;
    }
    catch (...) 
    {
        throw Poco::Exception
        (
            std::format
            (
                "Connection attempt to postgresql failed with host {0} and port {1}",
                host, port
            )
        );
    }

    return sessionPool;
} 

std::unique_ptr<RedisClientObjectPool> connectToRedis
(
    const std::string & host, 
    const std::string & port,
    const uint8_t       minSessions,
    const uint8_t       maxSessions
)
{
    using RedisClientPoolableObjectFactory = Poco::PoolableObjectFactory<Poco::Redis::Client, Poco::Redis::Client::Ptr>;
    
    std::unique_ptr<RedisClientObjectPool> redisPool = std::make_unique<RedisClientObjectPool>(
        RedisClientPoolableObjectFactory(std::format("{0}:{1}", host, port)), minSessions, maxSessions
    );

    try
    {
        Poco::Redis::PooledConnection pc(*redisPool);
        Poco::Redis::Array cmd;
        cmd << "PING";
        std::string result = static_cast<Poco::Redis::Client::Ptr>(pc)->execute<std::string>(cmd);
    }
    catch (...) 
    {
        throw Poco::Exception
        (
            std::format
            (
                "Connection attempt to redis failed with host {0} and port {1}",
                host, port
            )
        );
    }
    
    return redisPool;
} 

} // namespace RGT::Devkit
