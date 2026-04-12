#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/JSON/Object.h>

namespace RGT::Auth::Handlers
{

class ErrorHandler : public Poco::Net::HTTPRequestHandler
{
private:
    virtual void handleRequest(Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response)
    {
        Poco::JSON::Object::Ptr json = new Poco::JSON::Object;
        json->set("status", "eror");
        json->set("message", "Non-existent URL or bad method");

        response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        std::ostream & out = response.send();
        json->stringify(out);
    }
};

} // namespace RGT::Auth::Handlers
