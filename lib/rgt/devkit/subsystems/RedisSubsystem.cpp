#include <rgt/devkit/subsystems/RedisSubsystem.h>
#include <rgt/devkit/General.h>

#include <Poco/Util/Application.h>
#include <Poco/Redis/PoolableConnectionFactory.h>

namespace RGT::Devkit::Subsystems
{

const char * RedisSubsystem::name() const
{ return "RedisSubsystem"; }

RedisSubsystem::RedisClientObjectPool & RedisSubsystem::getPool()
{ return *redisPool_; }
      
void RedisSubsystem::initialize(Poco::Util::Application & app)
{  
    Poco::Util::LayeredConfiguration & cfg = app.config();

    std::optional<std::string> redisHost = RGT::Devkit::getEnvOrCfg("REDIS_HOST", "redis.host", cfg);
    if (not redisHost.has_value()) {
        throw std::runtime_error("The redis host is not set via environment variable (REDIS_HOST) or config (redis.host)");
    }
    std::optional<std::string> redisPort = RGT::Devkit::getEnvOrCfg("REDIS_PORT", "redis.port", cfg);
    if (not redisPort.has_value()) {
        throw std::runtime_error("The redis port is not set via environment variable (REDIS_HOST) or config (redis.host)");
    }

    redisPool_ = connectToRedis(redisHost.value(), redisPort.value(), 
        cfg.getUInt16("redis.min_sessions"), cfg.getUInt16("redis.max_sessions"));
}

void RedisSubsystem::uninitialize()
{ 
}

std::unique_ptr<RedisSubsystem::RedisClientObjectPool> RedisSubsystem::connectToRedis
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

} // namespace RGT::Devkit::Subsystems
