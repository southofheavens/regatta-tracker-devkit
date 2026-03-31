#ifndef __RABBITMQ_SUBSYSTEM_H__
#define __RABBITMQ_SUBSYSTEM_H__

#include <Poco/Util/Subsystem.h>
#include <Poco/Util/Application.h>

#include <SimpleAmqpClient/Channel.h>

namespace RGT::Devkit::Subsystems
{

class RabbitMQSubsystem : public Poco::Util::Subsystem
{
public:
    virtual const char * name() const final;

    AmqpClient::Channel & getChannel();

private:
    virtual void initialize(Poco::Util::Application & app) final;

	virtual void uninitialize() final;

private:
    AmqpClient::Channel::ptr_t amqpChannel_;
};

} // namespace RGT::Devkit::Subsystems

#endif // __RABBITMQ_SUBSYSTEM_H__
