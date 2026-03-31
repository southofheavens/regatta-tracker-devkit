#include <rgt/devkit/subsystems/RabbitMQSubsystem.h>

#include <rgt/devkit/General.h>
#include <rgt/devkit/AMQPWrappers.h>

#include <rabbitmq-c/tcp_socket.h>

namespace RGT::Devkit::Subsystems
{

const char * RabbitMQSubsystem::name() const
{ return "RabbitMQSubsystem"; }

AmqpClient::Channel & RabbitMQSubsystem::getChannel()
{ return *amqpChannel_; }

void RabbitMQSubsystem::initialize(Poco::Util::Application & app) 
{
    Poco::Util::LayeredConfiguration & cfg = app.config();

    std::optional<std::string> host = Devkit::getEnvOrCfg("RABBITMQ_HOST", "rabbitmq.host", cfg);
    if (not host.has_value()) {
        throw std::runtime_error("The host must be specified via environment variable or config");
    }

    std::optional<std::string> strPort = Devkit::getEnvOrCfg("RABBITMQ_PORT", "rabbitmq.port", cfg);
    if (not strPort.has_value()) {
        throw std::runtime_error("The port must be specified via environment variable or config");
    }
    uint16_t port;
    try {
        port = std::stoul(*strPort);
    }
    catch (const std::exception & e) {
        throw std::runtime_error("The port must be specified as an unsigned integer in the range 0 - 65535");
    }

    std::string vhost;
    try {
        vhost = cfg.getString("rabbitmq.vhost");
    }
    catch (const Poco::NotFoundException & e) {
        throw std::runtime_error("The vhost must be specified via config");
    }

    std::optional<std::string> username = Devkit::getEnv("RABBITMQ_USERNAME");
    if (not username.has_value()) {
        throw std::runtime_error("The username must be specified via environment variable");
    }

    std::optional<std::string> password = Devkit::getEnv("RABBITMQ_PASSWORD");
    if (not password.has_value()) {
        throw std::runtime_error("The password must be specified via environment variable");
    }

    // лучше это вынести в конфиг, но чтобы отобразить всевозможные настройки в конфиге,
    // уйдёт куча времени. я позволю себе сделать хардкод, но это очень ужасно и так делать не надо

    amqpChannel_ = AmqpClient::Channel::Create(*host, port, *username, *password, vhost);

    amqpChannel_->DeclareQueue("postprocessor_tasks", false, true, false, false);
    amqpChannel_->DeclareQueue("analytics_tasks", false, true, false, false);
    amqpChannel_->DeclareQueue("notifier_tasks", false, true, false, false);
}

void RabbitMQSubsystem::uninitialize()
{
}

} // namespace RGT::Devkit::Subsystems
