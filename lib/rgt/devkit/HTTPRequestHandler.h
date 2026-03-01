#ifndef __HTTP_REQUEST_HANDLER_H__
#define __HTTP_REQUEST_HANDLER_H__

#include <rgt/devkit/RGTException.h>
#include <rgt/devkit/JWTPayload.h>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/JSON/Object.h>

#include <any>

namespace RGT::Devkit
{

/// @brief Класс для обработки HTTP-запроса
/// @details Работа с HTTP-запросом проходит в три стадии: 
/// 1. Препроцессинг;
/// 2. Извлечение данных, необходимых для обработки запроса;
/// 3. Обработка запроса.
/// Для этого в классе предусмотрено 3 чисто-виртуальных метода, которые необходимо переопределить:
/// requestPreprocessing: предобработка запроса. Проверяется тип контента, размер контента и т.д.;
/// extractPayloadFromRequest: извлекает из запроса необходимые данные и возвращает их в виде структуры,
/// которую необходимо определить в классе-наследнике. Возвращаемое значение функции присваивается
/// переменной-члену класса payload_;
/// requestProcessing: обработка запроса с применением payload_.
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
        sendJsonResponse(response, "error", e.what());
    }
    catch (...)
    {
        response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        sendJsonResponse(response, "error", "Internal server error");
    }

protected:
    virtual void requestPreprocessing(Poco::Net::HTTPServerRequest & request) = 0;

    virtual std::any extractPayloadFromRequest(Poco::Net::HTTPServerRequest & request) = 0;

    virtual void requestProcessing(Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response) = 0;

protected:
    std::any payload_;

private:
    /// Большинство из этих статических методов по сути своей являются обёртками над функциями/методами
    /// из Poco. Мои обертки выбрасывают RGTException, который содержит текст и код ответа HTTP,
    /// которые, в свою очередь, мы можем отправить клиенту, чтобы он узнал причину,
    /// почему его запрос не был корректно обработан. Содержимое (метод what) всех остальных исключений
    /// клиент видеть не должен в целях безопасности работы сервера. Если вылетает такое исключение, клиенту будет 
    /// возвращён код 500 (Internal server error) и данная ошибка будет залогирована

    /// @brief Извлекает Bearer access-токен из HTTP-запроса
    /// @param request Ссылка на запрос
    /// @throw RGT::Devkit::RGTException если в запросе отсутствует заголовок Authorization 
    /// @throw RGT::Devkit::RGTException если тип токена не Bearer
    /// @throw RGT::Devkit::RGTException если токен не соответствует формату JWT (RFC 7519)
    /// @return Токен
    static std::string extractTokenFromRequest(Poco::Net::HTTPServerRequest & request);

    /// @brief Извлекает полезную нагрузку JWTPayload 
    /// @param token Токен
    /// @throw RGT::Devkit::RGTException если 
    /// @throw - Токен не соответствует формату JWT-токена
    /// @throw - У токена некорректная подпись
    /// @throw - Токен просрочен
    /// @throw - Sub нельзя перевести из std::string в uint64_t
    /// @return Полезная нагрузка
    static JWTPayload extractPayload(const std::string & token);
        
    /**
     * Извлекает из запроса JSON object и возвращает указатель на него
     */
    static Poco::JSON::Object::Ptr extractJsonObjectFromRequest(Poco::Net::HTTPServerRequest & req);

    /// @brief Проверяет, что длина контента из запроса не превышает maxContentLength
    /// @param request Ссылка на запрос
    /// @param maxContentLength Максимальная длина контента 
    /// @throw RGT::Devkit::RGTException если длина контента неизвестна или превышает установленный лимит
    static void checkContentLength(Poco::Net::HTTPServerRequest & request, const uint64_t & maxContentLength);

    /// @brief Проверяет, что длина контента из запроса не равна нулю
    /// @param request Ссылка на запрос
    /// @throw RGT::Devkit::RGTException если длина контента равна нулю
    static void checkContentLengthIsNull(Poco::Net::HTTPServerRequest & request);

    /// @brief Проверяет, что тип контента соответствует указанному
    /// @param request Ссылка на запрос
    /// @param contentType Тип контента
    /// @throw RGT::Devkit::RGTException если запрос не содержит тип контента или он не соответствует указанному
    static void checkContentType(Poco::Net::HTTPServerRequest & request, const std::string & contentType);

    /// @brief Извлекает из запроса значение, соответствующее заголовку
    /// @param request Ссылка на запрос
    /// @param header Заголовок
    /// @throw RGT::Devkit::RGTException если запрос не содержит указанный заголовок
    /// @return Ссылка на значение
    /// @warning После вызова деструктора для объекта request, ссылка, которая была возвращена из функции,
    /// станет висячей
    static const std::string & extractValueFromHeaders(Poco::Net::HTTPServerRequest & request, const std::string & header);

    /// @brief Извлекает из cookies значение, соответствующее ключу
    /// @param cookies Ссылка на cookies
    /// @param key Ключ
    /// @throw RGT::Devkit::RGTException если в cookies отсутствует указанный ключ
    /// @return Ссылка на значение
    /// @warning После вызова деструктора для объекта cookies, ссылка, которая была возвращена из функции,
    /// станет висячей
    static const std::string & extractValueFromCookies(Poco::Net::NameValueCollection & cookies, const std::string & key);

    /// @brief Извлекает из объекта JSON значение, соответствующее ключу
    /// @param json Указатель на объект JSON
    /// @param key Ключ
    /// @throw RGT::Devkit::RGTException если объект JSON не содержит указанный ключ
    /// @return Значение, соответствующее ключу
    static Poco::Dynamic::Var extractValueFromJson(Poco::JSON::Object::Ptr json, const std::string & key);

    /**
     * Отправляет клиенту ответ со статусом status и сообщением message 
     */
    static void sendJsonResponse(Poco::Net::HTTPServerResponse & res,
        const std::string & status, const std::string & message);

    /// @brief Извлекает из cookies запроса или объекта JSON (который взят из запроса!) refresh-токен
    /// @param request Ссылка на запрос
    /// @param json Указатель на указатель на json
    /// @throw RGT::Devkit::RGTException если в cookies запроса отсутствует refresh-токен и при попытке
    /// извлечь JSON из запроса (если *json == nullptr) произошла ошибка
    /// @throw RGT::Devkit::RGTException если в cookies запроса и объекте JSON отсутствует refresh-токен
    /// @return Рефреш-токен
    /// @warning Сначала пытается извлечь из cookies запроса, затем из json
    /// @note Если *json == nullptr, функция пытается извлечь json из запроса и присвоить значение *json
    static std::string extractRefreshFromRequest(Poco::Net::HTTPServerRequest & request,
        std::shared_ptr<Poco::JSON::Object::Ptr> json);

    /// @brief Извлекает из заголовка запроса или объекта JSON (который взят из запроса!) fingerprint
    /// @param request Ссылка на запрос
    /// @param json Указатель на указатель на json
    /// @throw RGT::Devkit::RGTException если в заголовках запроса отсутствует fingerprint и при попытке
    /// извлечь JSON из запроса (если *json == nullptr) произошла ошибка
    /// @throw RGT::Devkit::RGTException если в заголовках запроса и объекте JSON отсутствует fingerprint
    /// @return Fingerprint
    /// @warning Сначала пытается извлечь из заголовков запроса, затем из json
    /// @note Если *json == nullptr, функция пытается извлечь json из запроса и присвоить значение *json
    static std::string extractFingerprintFromRequest(Poco::Net::HTTPServerRequest & request,
        std::shared_ptr<Poco::JSON::Object::Ptr> json);
};

} // namespace RGT::Devkit

#endif // __HTTP_REQUEST_HANDLER_H__
