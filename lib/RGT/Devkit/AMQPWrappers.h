// #pragma once

// #include <rabbitmq-c/tcp_socket.h>
// #include <rabbitmq-c/amqp.h>

// #include <string>
// #include <memory>
// #include <unordered_map>
// #include <chrono>
// #include <mutex>

// namespace RGT::Devkit
// {

// class AMQPChannel;

// class AMQPConnection : std::enable_shared_from_this<AMQPConnection>
// {
// public:
//     AMQPConnection
//     (
//         const std::string & host,
//         const uint16_t port,
//         const std::string & vhost,
//         const std::string & username,
//         const std::string & password,
//         const uint16_t channelMax = AMQP_DEFAULT_MAX_CHANNELS,
//         const uint32_t frameMax = AMQP_DEFAULT_FRAME_SIZE,
//         const uint16_t heartbeat = 0
//     );

//     ~AMQPConnection();

//     void channelOpen(const std::string & channelID);
//     AMQPChannel & getChannel(const std::string & channelID);

// private:
//     amqp_connection_state_t connectionState_;
    
//     uint16_t nextChannelId_ = 1;
//     std::unordered_map<std::string, std::unique_ptr<AMQPChannel>> channels_;

//     std::mutex connectionMtx_;

//     friend AMQPChannel;
// };

// class AMQPChannel 
// {
// public:
//     ~AMQPChannel() = default;

//     AMQPConnection & getConnection();

//     /// @brief Включает режим подтверждения публикации
//     void confirmSelect( std::chrono::milliseconds timeout );

//     /// @brief Объявляет очередь в канале
//     void queueDeclare
//     (
//         const std::string & queue,
//         bool passive, 
//         bool durable, 
//         bool exclusive,
//         bool autoDelete,
//         amqp_table_t arguments = AMQP_EMPTY_TABLE
//     );

//     /// @brief Публикует сообщение
//     /// @note Если до этого был вызван метод confirmSelect, то
//     /// функция будет ожидать от брокера подтверждение о принятии сообщения.
//     /// Если таймаут истёк, получен NACK и т.д., будет выброшено исключение
//     void basicPublish
//     (
//         const std::string & exchange,
//         const std::string & routingKey,
//         bool mandatory, 
//         bool immediate, 
//         const amqp_basic_properties_t_ * properties, 
//         const std::string & body
//     );

    

// private:
//     AMQPChannel(std::shared_ptr<AMQPConnection> connection, amqp_channel_t channelID);

// private:
//     std::shared_ptr<AMQPConnection> connection_;
//     amqp_channel_t channelID_;

//     bool confirmSelectOn_ = false;
//     std::chrono::milliseconds confirmTimeout_;

//     friend AMQPConnection;
// };

// } // namespace RGT::Devkit
