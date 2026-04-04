#pragma once

#include <aws/s3/S3Client.h>

namespace Poco::Util { class AbstractConfiguration; } // namespace Poco::Util

namespace RGT::Devkit::Connections
{

/// @brief Создаёт и инициализирует соединение с S3 хранилищем
/// @return Указатель на S3Client
/// @throw std::runtime_error при ошибке подключения
std::unique_ptr<Aws::S3::S3Client> connectToS3
(
    const std::string & accessKeyId,
    const std::string & secretKey,
    const std::string & endpointOverride,
    const std::string & region, 
    const Aws::Http::Scheme & scheme,
    bool verifySsl,
    bool useVirtualAddressing,
    Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy signingPolicy = 
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never
);

/// @brief Создаёт и инициализирует соединение с S3 хранилищем, используя
/// параметры из конфига и переменных окружения
/// @return Указатель на S3Client
/// @throw std::runtime_error при ошибке подключения
std::unique_ptr<Aws::S3::S3Client> connectToS3
(
    const Poco::Util::AbstractConfiguration & cfg
);

} // namespace RGT::Devkit::Connections
