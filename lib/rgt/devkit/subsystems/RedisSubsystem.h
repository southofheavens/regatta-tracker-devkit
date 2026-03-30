#ifndef __REDIS_SUBSYSTEM_H__
#define __REDIS_SUBSYSTEM_H__

#include <Poco/Util/Subsystem.h>
#include <Poco/Redis/Client.h>
#include <Poco/Redis/PoolableConnectionFactory.h>
#include <Poco/Util/Application.h>

namespace RGT::Devkit::Subsystems
{

class RedisSubsystem : public Poco::Util::Subsystem
{
public:
    using RedisClientObjectPool = Poco::ObjectPool
    <
        Poco::Redis::Client, 
        Poco::Redis::Client::Ptr, 
        Poco::PoolableObjectFactory<Poco::Redis::Client, Poco::Redis::Client::Ptr>
    >;

    virtual const char * name() const final;

    RedisClientObjectPool & getPool();

private:        
    virtual void initialize(Poco::Util::Application & app) final;

	virtual void uninitialize() final;

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

private:
    std::unique_ptr<RedisClientObjectPool> redisPool_;
};

} // namespace RGT::Devkit::Subsystems

#endif // __REDIS_SUBSYSTEM_H__
