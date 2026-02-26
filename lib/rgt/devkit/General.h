#ifndef __GENERAL_H__
#define __GENERAL_H__

#include <exception>
#include <string>

#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/JSON/Object.h>

namespace RGT::Devkit
{

/**
 * Отправляет клиенту ответ со статусом status и сообщением message 
 */
void sendJsonResponse(Poco::Net::HTTPServerResponse & res,
    const std::string & status, const std::string & message);
    
/**
 * Извлекает из запроса JSON object и возвращает указатель на него
 */
Poco::JSON::Object::Ptr extractJsonObjectFromRequest(Poco::Net::HTTPServerRequest & req);

} // namespace RGT::Devkit

#endif // __GENERAL_H__
