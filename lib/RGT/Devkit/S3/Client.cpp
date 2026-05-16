#include <RGT/Devkit/S3/Client.h>
#include <RGT/Devkit/General.h>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/StreamCopier.h>
#include <Poco/Timestamp.h>
#include <Poco/URI.h>
#include <Poco/Util/AbstractConfiguration.h>

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <format>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{

struct HostPort
{
    std::string host;
    Poco::UInt16 port = 0;
};

std::string toLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool minioSchemeIsHttps()
{
    std::optional<std::string> raw = RGT::Devkit::getEnv("MINIO_SCHEME");
    if (not raw.has_value()) {
        return false;
    }
    return toLower(*raw) == "https";
}

std::pair<std::string, bool> normalizedEndpoint(std::string endpoint)
{
    bool https = minioSchemeIsHttps();
    if (endpoint.starts_with("https://")) {
        endpoint.erase(0, 8);
        https = true;
    }
    else if (endpoint.starts_with("http://")) {
        endpoint.erase(0, 7);
        https = false;
    }
    return {std::move(endpoint), https};
}

HostPort parseHostPort(const std::string & endpoint, bool https)
{
    const auto colon = endpoint.rfind(':');
    if (colon == std::string::npos) {
        return {endpoint, static_cast<Poco::UInt16>(https ? 443 : 80)};
    }
    return {
        endpoint.substr(0, colon),
        static_cast<Poco::UInt16>(std::stoul(endpoint.substr(colon + 1)))
    };
}

std::string sha256Hex(const std::string & data)
{
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen = 0;
    EVP_Digest(data.data(), data.size(), hash, &hashLen, EVP_sha256(), nullptr);
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < hashLen; ++i) {
        oss << std::setw(2) << static_cast<int>(hash[i]);
    }
    return oss.str();
}

std::vector<unsigned char> hmacSha256
(
    const std::vector<unsigned char> & key,
    const std::string & data
)
{
    unsigned int len = EVP_MAX_MD_SIZE;
    std::vector<unsigned char> out(EVP_MAX_MD_SIZE);
    HMAC(EVP_sha256(),
        key.data(),
        static_cast<int>(key.size()),
        reinterpret_cast<const unsigned char *>(data.data()),
        data.size(),
        out.data(),
        &len);
    out.resize(len);
    return out;
}

std::vector<unsigned char> hmacSha256(const std::string & key, const std::string & data)
{
    return hmacSha256(std::vector<unsigned char>(key.begin(), key.end()), data);
}

std::string amzDateUtc()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tmUtc{};
    gmtime_r(&t, &tmUtc);
    std::ostringstream oss;
    oss << std::put_time(&tmUtc, "%Y%m%dT%H%M%SZ");
    return oss.str();
}

std::string dateStampFromAmzDate(const std::string & amzDate)
{
    return amzDate.substr(0, 8);
}

std::string uriEncode(const std::string & value)
{
    std::string encoded;
    Poco::URI::encode(value, "", encoded);
    return encoded;
}

std::string canonicalUriPath(const std::string & path)
{
    if (path == "/") {
        return "/";
    }
    if (path.empty() or path.front() != '/') {
        throw std::invalid_argument("S3 path must start with '/'");
    }

    std::ostringstream canonical;
    canonical << '/';
    std::size_t pos = 1;
    bool first = true;
    while (pos < path.size())
    {
        const std::size_t next = path.find('/', pos);
        const std::size_t end = next == std::string::npos ? path.size() : next;
        if (not first) {
            canonical << '/';
        }
        first = false;
        canonical << uriEncode(path.substr(pos, end - pos));
        pos = next == std::string::npos ? path.size() : next + 1;
    }
    return canonical.str();
}

std::string buildCanonicalQuery(const std::map<std::string, std::string> & query)
{
    if (query.empty()) {
        return "";
    }
    std::ostringstream canonical;
    bool first = true;
    for (const auto & [key, value] : query)
    {
        if (not first) {
            canonical << '&';
        }
        first = false;
        canonical << uriEncode(key) << '=' << uriEncode(value);
    }
    return canonical.str();
}

std::string signAuthorization
(
    const RGT::Devkit::S3::ClientConfig & config,
    const std::string & method,
    const std::string & canonicalUri,
    const std::map<std::string, std::string> & query,
    const std::map<std::string, std::string> & headers,
    const std::string & payloadHash,
    const std::string & amzDate
)
{
    const std::string dateStamp = dateStampFromAmzDate(amzDate);
    const std::string credentialScope = std::format("{}/{}/s3/aws4_request", dateStamp, config.region);

    std::ostringstream canonicalHeaders;
    std::ostringstream signedHeadersList;
    bool first = true;
    for (const auto & [name, value] : headers)
    {
        canonicalHeaders << name << ':' << value << '\n';
        if (not first) {
            signedHeadersList << ';';
        }
        first = false;
        signedHeadersList << name;
    }

    const std::string canonicalRequest = std::format(
        "{}\n{}\n{}\n{}\n{}\n{}",
        method,
        canonicalUri,
        buildCanonicalQuery(query),
        canonicalHeaders.str(),
        signedHeadersList.str(),
        payloadHash);

    const std::string stringToSign = std::format(
        "AWS4-HMAC-SHA256\n{}\n{}\n{}",
        amzDate,
        credentialScope,
        sha256Hex(canonicalRequest));

    auto kDate = hmacSha256("AWS4" + config.secretAccessKey, dateStamp);
    auto kRegion = hmacSha256(kDate, config.region);
    auto kService = hmacSha256(kRegion, "s3");
    auto kSigning = hmacSha256(kService, "aws4_request");
    const auto signatureBytes = hmacSha256(kSigning, stringToSign);

    std::ostringstream signatureHex;
    signatureHex << std::hex << std::setfill('0');
    for (unsigned char b : signatureBytes) {
        signatureHex << std::setw(2) << static_cast<int>(b);
    }

    return std::format(
        "AWS4-HMAC-SHA256 Credential={}/{}, SignedHeaders={}, Signature={}",
        config.accessKeyId,
        credentialScope,
        signedHeadersList.str(),
        signatureHex.str());
}

void performSignedRequest
(
    const RGT::Devkit::S3::ClientConfig & config,
    const std::string & method,
    const std::string & path,
    const std::map<std::string, std::string> & query,
    const std::string * body,
    const std::string * contentType
)
{
    const auto [endpoint, https] = normalizedEndpoint(config.endpointHostPort);
    const HostPort hostPort = parseHostPort(endpoint, https);
    const std::string amzDate = amzDateUtc();
    const std::string payloadHash = body ? "UNSIGNED-PAYLOAD" : sha256Hex("");

    std::map<std::string, std::string> headers;
    headers.emplace("host", std::format("{}:{}", hostPort.host, hostPort.port));
    headers.emplace("x-amz-content-sha256", payloadHash);
    headers.emplace("x-amz-date", amzDate);
    if (contentType) {
        headers.emplace("content-type", *contentType);
    }

    const std::string authorization = signAuthorization(
        config, method, canonicalUriPath(path), query, headers, payloadHash, amzDate);
    headers.emplace("authorization", authorization);

    std::string requestPath = path;
    if (not query.empty())
    {
        requestPath.push_back('?');
        requestPath += buildCanonicalQuery(query);
    }

    std::unique_ptr<Poco::Net::HTTPClientSession> session;
    if (https) {
        session = std::make_unique<Poco::Net::HTTPSClientSession>(hostPort.host, hostPort.port);
    }
    else {
        session = std::make_unique<Poco::Net::HTTPClientSession>(hostPort.host, hostPort.port);
    }
    session->setTimeout(Poco::Timespan(60, 0));

    Poco::Net::HTTPRequest request{method, requestPath, Poco::Net::HTTPMessage::HTTP_1_1};
    request.setKeepAlive(false);
    for (const auto & [name, value] : headers) {
        request.set(name, value);
    }
    if (body) {
        request.setContentLength(static_cast<std::streamsize>(body->size()));
    }

    std::ostream & sendStream = session->sendRequest(request);
    if (body) {
        sendStream << *body;
        sendStream.flush();
    }

    Poco::Net::HTTPResponse response;
    std::istream & responseStream = session->receiveResponse(response);
    std::string responseBody;
    Poco::StreamCopier::copyToString(responseStream, responseBody);

    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK)
    {
        throw std::runtime_error(std::format(
            "S3 {} {} failed: HTTP {} {}",
            method,
            path,
            static_cast<int>(response.getStatus()),
            responseBody));
    }
}

} // namespace

namespace RGT::Devkit::S3
{

Client::Client(ClientConfig config)
    : config_{std::move(config)}
{
}

void Client::ping()
{
    performSignedRequest(config_, Poco::Net::HTTPRequest::HTTP_GET, "/", {}, nullptr, nullptr);
}

void Client::putObject
(
    const std::string & bucket,
    const std::string & objectKey,
    const std::string & body,
    const std::string & contentType
)
{
    const std::string path = std::format("/{}/{}", bucket, objectKey);
    performSignedRequest(
        config_,
        Poco::Net::HTTPRequest::HTTP_PUT,
        path,
        {},
        &body,
        &contentType);
}

std::unique_ptr<Client> makeClientFromEnv(const Poco::Util::AbstractConfiguration & cfg)
{
    std::optional<std::string> accessKeyId = RGT::Devkit::getEnv("MINIO_ACCESS_KEY_ID");
    if (not accessKeyId.has_value()) {
        throw std::runtime_error("MINIO_ACCESS_KEY_ID is not set");
    }
    std::optional<std::string> secretKey = RGT::Devkit::getEnv("MINIO_SECRET_KEY");
    if (not secretKey.has_value()) {
        throw std::runtime_error("MINIO_SECRET_KEY is not set");
    }
    std::optional<std::string> endpointOverride = RGT::Devkit::getEnvOrCfg(
        "MINIO_ENDPOINT_OVERRIDE",
        "minio.endpoint_override",
        cfg);
    if (not endpointOverride.has_value()) {
        throw std::runtime_error(
            "MINIO_ENDPOINT_OVERRIDE or minio.endpoint_override is not set");
    }

    auto [hostPort, https] = normalizedEndpoint(*endpointOverride);

    ClientConfig config{
        .accessKeyId = *accessKeyId,
        .secretAccessKey = *secretKey,
        .endpointHostPort = hostPort,
        .region = "us-east-1",
        .useHttps = https,
    };

    auto client = std::make_unique<Client>(std::move(config));
    client->ping();
    return client;
}

} // namespace RGT::Devkit::S3
