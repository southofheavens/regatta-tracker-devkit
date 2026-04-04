#include <RGT/Devkit/Connections/S3.h>
#include <RGT/Devkit/General.h>

#include <Poco/Util/AbstractConfiguration.h>

#include <aws/core/auth/AWSCredentials.h>

namespace RGT::Devkit::Connections
{

std::unique_ptr<Aws::S3::S3Client> connectToS3
(
    const std::string & accessKeyId,
    const std::string & secretKey,
    const std::string & endpointOverride,
    const std::string & region, 
    const Aws::Http::Scheme & scheme,
    bool verifySsl,
    bool useVirtualAddressing,
    Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy signingPolicy
)
{
    Aws::Auth::AWSCredentials credentials(Aws::String(accessKeyId.c_str()), Aws::String(secretKey.c_str()));

    Aws::Client::ClientConfiguration config;
    config.endpointOverride = endpointOverride;
    config.region = region;
    config.scheme = scheme;
    config.verifySSL = verifySsl;

    std::unique_ptr<Aws::S3::S3Client> clientPtr = 
        std::make_unique<Aws::S3::S3Client>(credentials, config, signingPolicy, useVirtualAddressing);

    Aws::S3::Model::ListBucketsRequest request;
    auto outcome = clientPtr->ListBuckets(request);
    if (!outcome.IsSuccess()) {
        throw std::runtime_error(outcome.GetError().GetMessage().c_str());
    }

    return clientPtr;
}

std::unique_ptr<Aws::S3::S3Client> connectToS3
(
    const Poco::Util::AbstractConfiguration & cfg
)
{
    std::optional<std::string> accessKeyId = RGT::Devkit::getEnv("MINIO_ACCESS_KEY_ID");
    if (not accessKeyId.has_value()) {
        throw std::runtime_error("The minio access key id is not set via environment variable (MINIO_ACCESS_KEY_ID)");
    }
    std::optional<std::string> secretKey = RGT::Devkit::getEnv("MINIO_SECRET_KEY");
    if (not secretKey.has_value()) {
        throw std::runtime_error("The minio secret key is not set via environment variable (MINIO_SECRET_KEY)");
    }
    std::optional<std::string> endpointOverride = RGT::Devkit::getEnvOrCfg("MINIO_ENDPOINT_OVERRIDE", 
        "minio.endpoint_override", cfg);
    if (not endpointOverride.has_value()) 
    {
        throw std::runtime_error("The minio endpoint override is not set via environment variable (MINIO_ENDPOINT_OVERRIDE) "
            "or config (minio.endpoint_override)");
    }

    return connectToS3(*accessKeyId, *secretKey, *endpointOverride, "", Aws::Http::Scheme::HTTPS, false, false);
}

} // namespace RGT::Devkit::Connections
