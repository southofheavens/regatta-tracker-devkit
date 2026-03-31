// #include <rgt/devkit/AMQPWrappers.h>

// #include <format>
// #include <sys/time.h>

// namespace RGT::Devkit
// {

// AMQPConnection::AMQPConnection
// (
//     const std::string & host,
//     const uint16_t port,
//     const std::string & vhost,
//     const std::string & username,
//     const std::string & password,
//     const uint16_t channelMax,
//     const uint32_t frameMax,
//     const uint16_t heartbeat
// )
// {
//     connectionState_ = amqp_new_connection();
//     if (connectionState_ == nullptr) {
//         throw std::runtime_error("amqp_new_connection error");
//     }
//     amqp_socket_t * socket = amqp_tcp_socket_new(connectionState_);
//     if (socket == nullptr) 
//     {
//         amqp_destroy_connection(connectionState_);
//         throw std::runtime_error("amqp_tcp_socket_new error");
//     }
//     int socketOpenResult = amqp_socket_open(socket, host.c_str(), port);
//     if (socketOpenResult != AMQP_STATUS_OK) 
//     {
//         amqp_destroy_connection(connectionState_);
//         throw std::runtime_error("amqp_socket_open error");
//     }
//     amqp_rpc_reply_t amqpLoginResult = amqp_login(connectionState_, vhost.c_str(), channelMax, 
//         frameMax, heartbeat, AMQP_SASL_METHOD_PLAIN, username.c_str(), password.c_str());
//     if (amqpLoginResult.reply_type != AMQP_RESPONSE_NORMAL) 
//     {
//         amqp_destroy_connection(connectionState_);
//         throw std::runtime_error("amqp_login error");
//     }
// }

// AMQPConnection::~AMQPConnection()
// {
//     amqp_connection_close(connectionState_, AMQP_REPLY_SUCCESS);
//     amqp_destroy_connection(connectionState_);
// }

// void AMQPConnection::channelOpen(const std::string & channelID)
// {
//     std::lock_guard<std::mutex> lg(connectionMtx_);

//     uint64_t newId = nextChannelId_++;
//     std::shared_ptr<RGT::Devkit::AMQPConnection> selfPtr = shared_from_this(); 

//     auto emplaceResult = channels_.try_emplace(channelID, 
//         std::unique_ptr<AMQPChannel>(new AMQPChannel(std::move(selfPtr), newId)));
//     if (not emplaceResult.second) {
//         throw std::runtime_error(std::format("channel with id {} already exists", channelID));
//     }
// }

// AMQPChannel & AMQPConnection::getChannel(const std::string & channelID)
// {
//     std::lock_guard<std::mutex> lg(connectionMtx_);

//     try {    
//         return *channels_.at(channelID);
//     }
//     catch (const std::exception & e) {
//         throw std::runtime_error(std::format("channel with id {} not exists", channelID));
//     }
// }



// AMQPConnection & AMQPChannel::getConnection()
// { 
//     if (connection_ == nullptr) [[unlikely]] {
//         throw std::runtime_error("connection ptr is null");
//     }
//     return *connection_;
// }

// void AMQPChannel::confirmSelect(std::chrono::milliseconds timeout)
// {
//     std::lock_guard<std::mutex> lg(connection_->connectionMtx_);

//     confirmSelectOn_ = true;
//     confirmTimeout_ = timeout;

//     amqp_confirm_select(connection_->connectionState_, channelID_);
//     amqp_rpc_reply_t rpcReply = amqp_get_rpc_reply(connection_->connectionState_);
//     if (rpcReply.reply_type != AMQP_RESPONSE_NORMAL) {
//         throw std::runtime_error("amqp_confirm_select error");
//     }
// }

// void AMQPChannel::queueDeclare
// (
//     const std::string & queue,
//     bool passive, 
//     bool durable, 
//     bool exclusive,
//     bool autoDelete,
//     amqp_table_t arguments
// )
// {
//     std::lock_guard<std::mutex> lg(connection_->connectionMtx_);

//     amqp_queue_declare
//     (
//         connection_->connectionState_, 
//         channelID_, 
//         amqp_cstring_bytes(queue.c_str()), 
//         passive, 
//         durable, 
//         exclusive, 
//         autoDelete, 
//         arguments
//     );
//     amqp_rpc_reply_t rpcReply = amqp_get_rpc_reply(connection_->connectionState_);
//     if (rpcReply.reply_type != AMQP_RESPONSE_NORMAL) {
//         throw std::runtime_error("amqp_queue_declare error");
//     }
// }

// void AMQPChannel::basicPublish
// (
//     const std::string & exchange,
//     const std::string & routingKey,
//     bool mandatory, 
//     bool immediate, 
//     const amqp_basic_properties_t_ * properties, 
//     const std::string & body
// )
// {
//     std::lock_guard<std::mutex> lg(connection_->connectionMtx_);

//     int publishResult = amqp_basic_publish
//     (
//         connection_->connectionState_,
//         channelID_, 
//         (exchange.empty() ? amqp_empty_bytes : amqp_cstring_bytes(exchange.c_str())),
//         (routingKey.empty() ? amqp_empty_bytes : amqp_cstring_bytes(routingKey.c_str())), 
//         mandatory, 
//         immediate, 
//         properties, 
//         (body.empty() ? amqp_empty_bytes : amqp_cstring_bytes(body.c_str()))
//     );
//     if (publishResult != AMQP_STATUS_OK) {
//         throw std::runtime_error("amqp_basic_publish error");
//     }

//     // Ожидаем подтверждение от брокера
//     std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
//     std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
//     while (currentTime - startTime < confirmTimeout_)
//     {
//         using namespace std::chrono;

//         auto remainingTime = confirmTimeout_ - (currentTime - startTime);
//         if (remainingTime <= std::chrono::steady_clock::duration::zero()) {
//             throw std::runtime_error("Publish confirmation timeout");
//         }
//         seconds seconds = duration_cast<std::chrono::seconds>(remainingTime);
//         microseconds microseconds = duration_cast<std::chrono::microseconds>(remainingTime);
//         struct timeval timeout = 
//         {
//             .tv_sec = seconds.count(),
//             .tv_usec = static_cast<int>(duration_cast<std::chrono::microseconds>(microseconds - seconds).count())
//         };

//         amqp_frame_t frame;
//         int status = amqp_simple_wait_frame_noblock(connection_->connectionState_, &frame, &timeout);
//         switch (status)
//         {
//             case AMQP_STATUS_TIMEOUT:
//                 throw std::runtime_error("the timeout was reached while waiting confirm from the broker");
//             case AMQP_STATUS_OK:
//                 break;
//             default:
//                 throw std::runtime_error("amqp_simple_wait_frame_noblock error");
//         }

//         if (frame.frame_type == AMQP_FRAME_METHOD)
//         {
//             if (frame.payload.method.id == AMQP_BASIC_ACK_METHOD) {
//                 break;
//             }
//             else if (frame.payload.method.id == AMQP_BASIC_NACK_METHOD) {
//                 throw std::runtime_error("message rejected by broker");
//             }
//         }

//         currentTime = std::chrono::steady_clock::now();
//     }
// }

// AMQPChannel::AMQPChannel(std::shared_ptr<AMQPConnection> connection, amqp_channel_t channelID)
//     : connection_{connection}
//     , channelID_{channelID}
// {
//     std::lock_guard<std::mutex> lg(connection_->connectionMtx_);

//     amqp_channel_open(connection->connectionState_, channelID);
//     amqp_rpc_reply_t rpcReply = amqp_get_rpc_reply(connection->connectionState_);
//     if (rpcReply.reply_type != AMQP_RESPONSE_NORMAL) {
//         throw std::runtime_error("amqp_channel_open error");
//     }
// }

// } // namespace RGT::Devkit
