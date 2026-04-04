#pragma once

#include <SimpleAmqpClient/Channel.h>

namespace Poco::Util { class AbstractConfiguration; } // namespace Poco::Util

namespace RGT::Devkit::Connections
{

/// @brief Создаёт и инициализирует соединение (канал) с RabbitMQ
/// @return Указатель на соединение
/// @throw std::runtime_error при ошибке подключения
AmqpClient::Channel::ptr_t connectToRabbitMQ
(
    const std::string & host,
    const uint16_t      port,
    const std::string & username,
    const std::string & password, 
    const std::string & vhost
); 

/// @brief Создает и инициализирует соединение (канал) с RabbitMQ, используя
/// параметры из конфига и переменных окружения
/// @return Указатель на соединение
/// @throw std::runtime_error при ошибке подключения
AmqpClient::Channel::ptr_t connectToRabbitMQ
(
    const Poco::Util::AbstractConfiguration & cfg
);

} // namespace RGT::Devkit::Connections
