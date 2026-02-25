#ifndef __REQUEST_PROCESSING_H__
#define __REQUEST_PROCESSING_H__

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>

namespace RGT::Devkit
{

/// @brief Проверяет, что длина контента из запроса не превышает maxContentLength
/// @param request Ссылка на запрос
/// @param maxContentLength Максимальная длина контента 
/// @throw RGT::Devkit::RGTException если длина контента неизвестна или превышает установленный лимит
void checkContentLength(const Poco::Net::HTTPServerRequest & request, const uint64_t & maxContentLength);

/// @brief Проверяет, что длина контента из запроса не равна нулю
/// @param request Ссылка на запрос
/// @throw RGT::Devkit::RGTException если длина контента равна нулю
void checkContentLengthIsNull(const Poco::Net::HTTPServerRequest & request);

/// @brief Проверяет, что тип контента соответствует указанному
/// @param request Ссылка на запрос
/// @param contentType Тип контента
/// @throw RGT::Devkit::RGTException если запрос не содержит тип контента или он не соответствует указанному
void checkContentType(const Poco::Net::HTTPServerRequest & request, const std::string & contentType);

/// @brief Извлекает из запроса значение, соответствующее заголовку
/// @param request Ссылка на запрос
/// @param header Заголовок
/// @throw RGT::Devkit::RGTException если запрос не содержит указанный заголовок
/// @return Ссылка на значение
/// @warning После вызова деструктора для объекта request, ссылка, которая была возвращена из функции,
/// станет висячей
const std::string & extractValueFromHeaders(const Poco::Net::HTTPServerRequest & request, const std::string & header);

/// @brief Извлекает из cookies значение, соответствующее ключу
/// @param cookies Ссылка на cookies
/// @param key Ключ
/// @throw RGT::Devkit::RGTException если в cookies отсутствует указанный ключ
/// @return Ссылка на значение
/// @warning После вызова деструктора для объекта cookies, ссылка, которая была возвращена из функции,
/// станет висячей
const std::string & extractValueFromCookies(const Poco::Net::NameValueCollection & cookies, const std::string & key);

/// @brief Извлекает из объекта JSON значение, соответствующее ключу
/// @param json Указатель на объект JSON
/// @param key Ключ
/// @throw RGT::Devkit::RGTException если объект JSON не содержит указанный ключ
/// @return Значение, соответствующее ключу
Poco::Dynamic::Var extractValueFromJson(const Poco::JSON::Object::Ptr json, const std::string & key);

} // namespace RGT::Devkit

#endif // __REQUEST_PROCESSING_H__
