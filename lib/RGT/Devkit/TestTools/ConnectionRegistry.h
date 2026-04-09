#pragma once

#include <RGT/Devkit/Connections/PostgreSQL.h>
#include <RGT/Devkit/Connections/Redis.h>
#include <RGT/Devkit/Connections/S3.h>
#include <RGT/Devkit/Connections/RabbitMQ.h>
#include <RGT/Devkit/General.h>
#include <RGT/Devkit/ProjectName.h>

#include <SimpleAmqpClient/Channel.h>

#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Util/JSONConfiguration.h>

#include <aws/core/Aws.h>

namespace RGT::Devkit::TestTools
{

class ConnectionRegistry
{
public:
    static ConnectionRegistry & instance()
    {
        static ConnectionRegistry connections;
        return connections;
    }

    Poco::Data::SessionPool & getPsqlPool()
    { return *psqlPool_; }

    RedisClientObjectPool & getRedisPool()
    { return *redisPool_; }

    Aws::S3::S3Client & getS3Client()
    { return *s3Client_; }

    AmqpClient::Channel & getRabbitMQChannel()
    { return *rabbitMQChannel_; }

    ~ConnectionRegistry()
    {
        Aws::ShutdownAPI(sdkOptions_);
        Poco::Data::PostgreSQL::Connector::unregisterConnector();
    }

private:
    ConnectionRegistry()
    {
        cfg_ = new Poco::Util::JSONConfiguration(RGT::Devkit::getProjectName());

        Poco::Data::PostgreSQL::Connector::registerConnector();
        Aws::InitAPI(sdkOptions_);

        psqlPool_ = Connections::connectToPsql(*cfg_);
        redisPool_ = Connections::connectToRedis(*cfg_);
        s3Client_ = Connections::connectToS3(*cfg_);
        rabbitMQChannel_ = Connections::connectToRabbitMQ(*cfg_);
    }

private:    
    std::unique_ptr<Poco::Data::SessionPool> psqlPool_;
    std::unique_ptr<RedisClientObjectPool>   redisPool_;
    std::unique_ptr<Aws::S3::S3Client>       s3Client_;
    AmqpClient::Channel::ptr_t               rabbitMQChannel_;

    Poco::Util::JSONConfiguration::Ptr cfg_;
    Aws::SDKOptions sdkOptions_;
};

} // namespace RGT::Devkit::TestTools
