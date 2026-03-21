#include <rgt/devkit/subsystems/S3Subsystem.h>
#include <rgt/devkit/General.h>

#include <Poco/Util/Application.h>

#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>

namespace RGT::Devkit::Subsystems
{

const char * S3Subsystem::name() const
{ return "S3Subsystem"; }

Aws::S3::S3Client & S3Subsystem::getS3Client()
{ return *s3Client_; }
      
void S3Subsystem::initialize(Poco::Util::Application & app)
{  
    Aws::InitAPI(sdkOptions_);

    Poco::Util::LayeredConfiguration & cfg = app.config();

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

    s3Client_ = connectToS3(*accessKeyId, *secretKey, *endpointOverride, "", Aws::Http::Scheme::HTTPS, false, false);
}

void S3Subsystem::uninitialize()
{ Aws::ShutdownAPI(sdkOptions_); }

std::unique_ptr<Aws::S3::S3Client> S3Subsystem::connectToS3
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

} // namespace RGT::Devkit::Subsystems
