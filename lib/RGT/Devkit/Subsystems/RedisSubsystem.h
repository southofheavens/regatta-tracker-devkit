#pragma once

#include <Poco/Util/Subsystem.h>
#include <Poco/Redis/Client.h>
#include <Poco/Redis/PoolableConnectionFactory.h>
#include <Poco/Util/Application.h>

namespace RGT::Devkit::Subsystems
{

class RedisSubsystem : public Poco::Util::Subsystem
{
public:
    using RedisClientObjectPool = Poco::ObjectPool<Poco::Redis::Client, Poco::Redis::Client::Ptr, 
        Poco::PoolableObjectFactory<Poco::Redis::Client, Poco::Redis::Client::Ptr>>;

    virtual const char * name() const final;

    RedisClientObjectPool & getPool();

private:        
    virtual void initialize(Poco::Util::Application & app) final;

	virtual void uninitialize() final;

private:
    std::unique_ptr<RedisClientObjectPool> redisPool_;
};

} // namespace RGT::Devkit::Subsystems
