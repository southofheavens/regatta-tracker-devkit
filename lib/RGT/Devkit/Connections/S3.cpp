#include <RGT/Devkit/Connections/S3.h>
#include <RGT/Devkit/General.h>

#include <Poco/Util/AbstractConfiguration.h>

#include <aws/core/auth/AWSCredentials.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

namespace
{

Aws::Http::Scheme minioSchemeFromEnv()
{
    std::optional<std::string> raw = RGT::Devkit::getEnv("MINIO_SCHEME");
    if (not raw.has_value()) {
        return Aws::Http::Scheme::HTTP;
    }
    std::string lowered = *raw;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(),
        [](unsigned char c)
    {
        return static_cast<char>(std::tolower(c));
    });

    return lowered == "https" ? Aws::Http::Scheme::HTTPS : Aws::Http::Scheme::HTTP;
}

std::pair<std::string, Aws::Http::Scheme> normalizedHostPortAndScheme
(
    std::string endpoint,
    Aws::Http::Scheme envScheme
)
{
    if (endpoint.starts_with("https://")) {
        endpoint.erase(0, 8);
        return {std::move(endpoint), Aws::Http::Scheme::HTTPS};
    }
    if (endpoint.starts_with("http://")) {
        endpoint.erase(0, 7);
        return {std::move(endpoint), Aws::Http::Scheme::HTTP};
    }
    return {std::move(endpoint), envScheme};
}

} // namespace

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

    const Aws::Http::Scheme envScheme = minioSchemeFromEnv();
    auto [hostPort, scheme] = normalizedHostPortAndScheme(*endpointOverride, envScheme);
    constexpr bool verifySsl = false;

    return connectToS3(*accessKeyId, *secretKey, hostPort, "", scheme, verifySsl, false,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never);
}

} // namespace RGT::Devkit::Connections
