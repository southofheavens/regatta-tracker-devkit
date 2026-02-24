#include <rgt/devkit/RequestProcessing.h>
#include <rgt/devkit/RGTException.h>

#include <format>

namespace RGT::Devkit
{

void checkContentLength(const Poco::Net::HTTPServerRequest & request, const uint64_t & maxContentLength)
{
    if (request.getContentLength() == Poco::Net::HTTPMessage::UNKNOWN_CONTENT_LENGTH) {
        throw RGT::Devkit::RGTException("Content length is unknown", 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
    }

    if (request.getContentLength64() > maxContentLength) {
        throw RGT::Devkit::RGTException(std::format("Content length must not exceed {}", maxContentLength),
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }
}

void checkContentLengthIsNull(const Poco::Net::HTTPServerRequest & request)
{
    if (request.getContentLength() == 0) {
        throw RGT::Devkit::RGTException("Content length is zero", 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
    }
}

void checkContentType(const Poco::Net::HTTPServerRequest & request, const std::string & contentType)
{
    const std::string & contentType = request.getContentType();

    if (contentType.empty()) {
        throw RGT::Devkit::RGTException("Expected to receive 'Content-Type' header", 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
    }

    if (contentType.find(contentType) == std::string::npos) {
        throw RGT::Devkit::RGTException(std::format("Content-Type must be {}", contentType), 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
    }
}

const std::string & extractValueByHeader(const Poco::Net::HTTPServerRequest & request, const std::string & header)
{
    try {
        return request.get(header);
    }
    catch (...) {
        throw RGT::Devkit::RGTException(std::format("Expected to receive '{}' header", header), 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST); 
    }
}

Poco::Dynamic::Var extractValueByKey(const Poco::JSON::Object::Ptr json, const std::string & key)
{
    try {
        return json->get(key);
    }
    catch (...) {
        throw RGT::Devkit::RGTException(std::format("Expected to receive '{}' json field", key), 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST); 
    }
}

const std::string & extractValueFromCookies(const Poco::Net::NameValueCollection & cookies, const std::string & key)
{
    try {
        return cookies[key];
    }
    catch (...) {
        throw RGT::Devkit::RGTException(std::format("Expected to receive '{}' field in cookies", key), 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST); 
    }
}

} // namespace RGT::Devkit
