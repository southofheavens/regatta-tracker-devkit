#include <rgt/devkit/General.h>
#include <rgt/devkit/RGTException.h>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

namespace RGT::Devkit
{

void sendJsonResponse(Poco::Net::HTTPServerResponse& res,
    const std::string& status, const std::string& message)
{
    Poco::JSON::Object json;
    json.set("status", status);
    json.set("message", message);

    std::ostream& out = res.send();
    json.stringify(out);
}

Poco::JSON::Object::Ptr extractJsonObjectFromRequest(Poco::Net::HTTPServerRequest & req)
{    
    Poco::JSON::Parser parser;

    Poco::Dynamic::Var result;
    try {
        result = parser.parse(req.stream());
    }
    catch (...) {
        throw RGT::Devkit::RGTException("Received invalid json", 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
    }

    if (result.type() != typeid(Poco::JSON::Object::Ptr)) {
        throw RGT::Devkit::RGTException("Expected JSON object, not array", 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
    }

    return result.extract<Poco::JSON::Object::Ptr>();
}

} // namespace RGT::Devkit
