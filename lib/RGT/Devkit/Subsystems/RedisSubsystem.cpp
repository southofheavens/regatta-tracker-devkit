#include <RGT/Devkit/Subsystems/RedisSubsystem.h>
#include <RGT/Devkit/General.h>
#include <RGT/Devkit/Connections/Redis.h>

namespace RGT::Devkit::Subsystems
{

const char * RedisSubsystem::name() const
{ return "RedisSubsystem"; }

RedisSubsystem::RedisClientObjectPool & RedisSubsystem::getPool()
{ return *redisPool_; }
      
void RedisSubsystem::initialize(Poco::Util::Application & app)
{  
    Poco::Util::LayeredConfiguration & cfg = app.config();

    redisPool_ = Connections::connectToRedis(cfg);
}

void RedisSubsystem::uninitialize()
{ 
}

} // namespace RGT::Devkit::Subsystems
