#pragma once

#include <RGT/Devkit/S3/Client.h>

#include <memory>

namespace Poco::Util { class AbstractConfiguration; } // namespace Poco::Util

namespace RGT::Devkit::Connections
{

/// @brief Создаёт клиент MinIO/S3 и проверяет ListBuckets.
std::unique_ptr<S3::Client> connectToS3(const Poco::Util::AbstractConfiguration & cfg);

} // namespace RGT::Devkit::Connections
