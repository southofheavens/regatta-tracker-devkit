#ifndef __TOKENS_H__
#define __TOKENS_H__

#include <string>
#include <chrono>

#include <Poco/Net/HTTPServerRequest.h>

namespace RGT::Devkit::Tokens
{

/**
 * @brief Полезная нагрузка access токена
 */
struct Payload 
{
    uint64_t             sub;
    std::string          role;
    std::chrono::seconds exp;
};

/**
 * @brief Извлекает полезную нагрузку из access токена
 * @param token Токен
 * @throw std::runtime_error если получен невалиданый токен
 * @return Полезная нагрузка
 */
Payload extractPayload(const std::string& token);

// Извлекает Bearer access-токен из запроса
// Функция ответственна за обработку ошибок при извлечении токена
// 
// Если в запросе отсутствует заголовок Authorization 
// или тип токена не Bearer 
// или токен невалидный (неверная подпись || просрочен),
// то будет выброшено исключение RGTException
std::string extractTokenFromRequest(Poco::Net::HTTPServerRequest & request);

} // namespace RGT::Devkit::Tokens

#endif // __TOKENS_H__

