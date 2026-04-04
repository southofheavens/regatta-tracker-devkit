#include <RGT/Devkit/Connections/RabbitMQ.h>
#include <RGT/Devkit/General.h>

#include <Poco/Util/AbstractConfiguration.h>

namespace RGT::Devkit::Connections
{

AmqpClient::Channel::ptr_t connectToRabbitMQ
(
    const std::string & host,
    const uint16_t      port,
    const std::string & username,
    const std::string & password, 
    const std::string & vhost
)
{
    AmqpClient::Channel::OpenOpts opts;
    opts.host = host;
    opts.port = port;
    opts.auth = AmqpClient::Channel::OpenOpts::BasicAuth(username, password);
    opts.vhost = vhost;

    AmqpClient::Channel::ptr_t channel;
    try {
        channel = AmqpClient::Channel::Open(opts);
    }
    catch (const std::exception & e) 
    {
        throw std::runtime_error(std::format("error while opening rabbitmq channel with host = {}, port = {}",
            host, port));
    }

    // TODO убрать хардкод
    channel->DeclareQueue("postprocessor_tasks", false, true, false, false);
    channel->DeclareQueue("analytics_tasks", false, true, false, false);
    channel->DeclareQueue("notifier_tasks", false, true, false, false);

    return channel;
}

AmqpClient::Channel::ptr_t connectToRabbitMQ
(
    const Poco::Util::AbstractConfiguration & cfg
)
{
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

    return connectToRabbitMQ(*host, port, *username, *password, vhost);
}

} // namespace RGT::Devkit::Connections
