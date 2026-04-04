#include <RGT/Devkit/Subsystems/RabbitMQSubsystem.h>
#include <RGT/Devkit/General.h>
#include <RGT/Devkit/Connections/RabbitMQ.h>

namespace RGT::Devkit::Subsystems
{

const char * RabbitMQSubsystem::name() const
{ return "RabbitMQSubsystem"; }

AmqpClient::Channel & RabbitMQSubsystem::getChannel()
{ return *amqpChannel_; }

void RabbitMQSubsystem::initialize(Poco::Util::Application & app) 
{
    Poco::Util::LayeredConfiguration & cfg = app.config();

    amqpChannel_ = Connections::connectToRabbitMQ(cfg);
}

void RabbitMQSubsystem::uninitialize()
{
}

} // namespace RGT::Devkit::Subsystems
