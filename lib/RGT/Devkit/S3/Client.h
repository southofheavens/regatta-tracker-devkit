#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace Poco::Util { class AbstractConfiguration; } // namespace Poco::Util

namespace RGT::Devkit::S3
{

struct ClientConfig
{
    std::string accessKeyId;
    std::string secretAccessKey;
    std::string endpointHostPort;
    std::string region = "us-east-1";
    bool useHttps = false;
};

class Client
{
public:
    explicit Client(ClientConfig config);

    /// Проверка доступности MinIO (ListBuckets).
    void ping();

    void putObject
    (
        const std::string & bucket,
        const std::string & objectKey,
        const std::string & body,
        const std::string & contentType
    );

private:
    ClientConfig config_;
};

std::unique_ptr<Client> makeClientFromEnv(const Poco::Util::AbstractConfiguration & cfg);

} // namespace RGT::Devkit::S3
