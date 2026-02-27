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

private:
    /// @brief Полезная нагрузка access токена
    struct JWTPayload 
    {
        uint64_t             sub;
        std::string          role;
        std::chrono::seconds exp;
    };

    /// @brief Извлекает Bearer access-токен из HTTP-запроса
    /// @param request Ссылка на запрос
    /// @throw RGT::Devkit::RGTException если в запросе отсутствует заголовок Authorization 
    /// @throw RGT::Devkit::RGTException если тип токена не Bearer
    /// @return Токен
    /// @warning Не проверяет токен на соответствие формату JWT. Если запрос содержит
    /// 'Authorization: Bearer abracadabra', функция вернет 'abracadabra' и не выбросит исключение
    static std::string extractTokenFromRequest(Poco::Net::HTTPServerRequest & request);

    /// @brief Извлекает полезную нагрузку JWTPayload и проводит проверку,
    /// что данные, извлеченные из токена, корректны
    /// @param token Токен
    /// @throw RGT::Devkit::RGTException если токен не соответствует формату JWT-токена
    /// @throw RGT::Devkit::RGTException если sub не является числом
    /// @throw RGT::Devkit::RGTException если роль не существует
    /// @return Полезная нагрузка
    static JWTPayload extractPayload(const std::string & token);

    /// @brief Проверяет, что длина контента из запроса не превышает maxContentLength
    /// @param request Ссылка на запрос
    /// @param maxContentLength Максимальная длина контента 
    /// @throw RGT::Devkit::RGTException если длина контента неизвестна или превышает установленный лимит
    static void checkContentLength(const Poco::Net::HTTPServerRequest & request, const uint64_t & maxContentLength);

    /// @brief Проверяет, что длина контента из запроса не равна нулю
    /// @param request Ссылка на запрос
    /// @throw RGT::Devkit::RGTException если длина контента равна нулю
    static void checkContentLengthIsNull(const Poco::Net::HTTPServerRequest & request);

    /// @brief Проверяет, что тип контента соответствует указанному
    /// @param request Ссылка на запрос
    /// @param contentType Тип контента
    /// @throw RGT::Devkit::RGTException если запрос не содержит тип контента или он не соответствует указанному
    static void checkContentType(const Poco::Net::HTTPServerRequest & request, const std::string & contentType);

    /// @brief Извлекает из запроса значение, соответствующее заголовку
    /// @param request Ссылка на запрос
    /// @param header Заголовок
    /// @throw RGT::Devkit::RGTException если запрос не содержит указанный заголовок
    /// @return Ссылка на значение
    /// @warning После вызова деструктора для объекта request, ссылка, которая была возвращена из функции,
    /// станет висячей
    static const std::string & extractValueFromHeaders(const Poco::Net::HTTPServerRequest & request, const std::string & header);

    /// @brief Извлекает из cookies значение, соответствующее ключу
    /// @param cookies Ссылка на cookies
    /// @param key Ключ
    /// @throw RGT::Devkit::RGTException если в cookies отсутствует указанный ключ
    /// @return Ссылка на значение
    /// @warning После вызова деструктора для объекта cookies, ссылка, которая была возвращена из функции,
    /// станет висячей
    static const std::string & extractValueFromCookies(const Poco::Net::NameValueCollection & cookies, const std::string & key);

    /// @brief Извлекает из объекта JSON значение, соответствующее ключу
    /// @param json Указатель на объект JSON
    /// @param key Ключ
    /// @throw RGT::Devkit::RGTException если объект JSON не содержит указанный ключ
    /// @return Значение, соответствующее ключу
    static Poco::Dynamic::Var extractValueFromJson(const Poco::JSON::Object::Ptr json, const std::string & key);
};

} // namespace RGT::Devkit

#endif // __HTTP_REQUEST_HANDLER_H__
