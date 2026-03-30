#ifndef __RABBITMQ_SUBSYSTEM_H__
#define __RABBITMQ_SUBSYSTEM_H__

#include <Poco/Util/Subsystem.h>
#include <Poco/Util/Application.h>

#include <rabbitmq-c/amqp.h>

namespace RGT::Devkit::Subsystems
{

class RabbitMQSubsystem : public Poco::Util::Subsystem
{
public:
    virtual const char * name() const final;

    struct AmqpConnection
    {
        amqp_connection_state_t connection;
        amqp_channel_t          channel;
    };
    AmqpConnection & getAmqpConnection(const std::string & connectionId);

private:
    virtual void initialize(Poco::Util::Application & app) final;

	virtual void uninitialize() final;

private:
    std::map<std::string, AmqpConnection> amqpConnections_;
};

} // namespace RGT::Devkit::Subsystems

#endif // __RABBITMQ_SUBSYSTEM_H__
