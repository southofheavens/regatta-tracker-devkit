#include <RGT/Devkit/Connections/Redis.h>
#include <RGT/Devkit/General.h>

#include <Poco/Util/AbstractConfiguration.h>

namespace RGT::Devkit::Connections
{

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
        throw std::runtime_error
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

std::unique_ptr<RedisClientObjectPool> connectToRedis
(
    const Poco::Util::AbstractConfiguration & cfg
)
{
    std::optional<std::string> redisHost = RGT::Devkit::getEnvOrCfg("REDIS_HOST", "redis.host", cfg);
    if (not redisHost.has_value()) {
        throw std::runtime_error("The redis host is not set via environment variable (REDIS_HOST) or config (redis.host)");
    }
    std::optional<std::string> redisPort = RGT::Devkit::getEnvOrCfg("REDIS_PORT", "redis.port", cfg);
    if (not redisPort.has_value()) {
        throw std::runtime_error("The redis port is not set via environment variable (REDIS_HOST) or config (redis.host)");
    }

    return connectToRedis(redisHost.value(), redisPort.value(), cfg.getUInt16("redis.min_sessions"), 
        cfg.getUInt16("redis.max_sessions"));
}

} // namespace RGT::Devkit::Connections
