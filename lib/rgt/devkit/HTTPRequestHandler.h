#ifndef __HTTP_REQUEST_HANDLER_H__
#define __HTTP_REQUEST_HANDLER_H__

#include <rgt/devkit/General.h>
#include <rgt/devkit/RGTException.h>

#include <Poco/Net/HTTPRequestHandler.h>

#include <any>

namespace RGT::Devkit
{

class HTTPRequestHandler : public Poco::Net::HTTPRequestHandler
{
private:
    virtual void handleRequest(Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response) final
    try
    {
        requestPreprocessing(request);
        payload_ = extractPayloadFromRequest(request);
        requestProcessing(request, response);
    }
    catch (const RGT::Devkit::RGTException & e)
    {
        response.setStatusAndReason(e.status());
        RGT::Devkit::sendJsonResponse(response, "error", e.what());
    }
    catch (...)
    {
        response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        RGT::Devkit::sendJsonResponse(response, "error", "Internal server error");
    }

protected:
    virtual void requestPreprocessing(Poco::Net::HTTPServerRequest & request) = 0;

    virtual std::any extractPayloadFromRequest(Poco::Net::HTTPServerRequest & request) = 0;

    virtual void requestProcessing(Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response) = 0;

protected:
    std::any payload_;
};

} // namespace RGT::Devkit

#endif // __HTTP_REQUEST_HANDLER_H__
