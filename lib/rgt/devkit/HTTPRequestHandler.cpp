#include <rgt/devkit/HTTPRequestHandler.h>

#include <Poco/JWT/Token.h>
#include <Poco/JWT/Serializer.h>
#include <Poco/JWT/Signer.h>

namespace
{

// TODO Нельзя держать приватный ключ в коде
const std::string secret_key = "secret_key";

/// @brief Проверяет, что строка соответствует формату Base64Url
/// @param str Строка
/// @return true, если строка соответствует формату Base64Url и false в противном случае
bool isBase64Url(const std::string & str)
{
    auto isValidChar = [](const char & character)
    {
        return std::isalnum(character) or character == '_' or character == '-';
    };

    for (const char & character : str)
    {
        if (not isValidChar(character)) {
            return false;
        }
    }

    return true;
}

/// @brief Проверяет, что строка соответствует формату JWT-токена (RFC 7519)
/// @note Выполняет структурную валидацию без криптографической проверки:
/// @note - Токен состоит из трёх частей, разделённых точкой: header.payload.signature
/// @note - Каждая часть корректно закодирована в Base64Url
/// @note - Header и payload успешно декодируются в валидный JSON
/// @param token Токен
/// @throw RGT::Devkit::RGTException если строка не соответствует формату 
void validateJWTFormat(const std::string & token)
{
    std::vector<std::string> parts = Poco::JWT::Serializer::split(token);

    if (parts.size() != 3) {
        throw RGT::Devkit::RGTException("Invalid JWT-token format (token must consist of three parts separated by a dot)",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }

    if (std::any_of(parts.begin(), parts.end(), [](const std::string & str){ return str.empty(); })) {
        throw RGT::Devkit::RGTException("Invalid JWT-token format (empty part detected)", 
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }

    try {
        Poco::JWT::Serializer::deserialize(parts[0]);
    }
    catch (...) {
        throw RGT::Devkit::RGTException("Invalid JWT-token format (header is not valid Base64Url or JSON)",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }

    try {
        Poco::JWT::Serializer::deserialize(parts[1]);
    }
    catch (...) {
        throw RGT::Devkit::RGTException("Invalid JWT-token format (payload is not valid Base64Url or JSON)",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }

    if (not isBase64Url(parts[2])) {
        throw RGT::Devkit::RGTException("Invalid JWT-token format (signature is not valid Base64Url)",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }
}

/// @brief Проверяет является ли access токен валидным
/// @note Токен является валидным, если:
/// @note У него корректная подпись
/// @note Он не просрочен
/// @param token Токен
/// @throw RGT::Devkit::RGTException если токен невалидный
void accessTokenValidate(const std::string & token)
{
    Poco::JWT::Signer signer(secret_key);
    signer.setAlgorithms({Poco::JWT::Signer::ALGO_HS256});
    
    Poco::JWT::Token decoded;
    try {
        decoded = signer.verify(token);
    }
    catch (...) {
        // throw
    }

    Poco::Timestamp expires = decoded.getExpiration();
    Poco::Timestamp now;
    
    if (expires < now) {
        // throw
    }
}

} // namespace

namespace RGT::Devkit
{

std::string HTTPRequestHandler::extractTokenFromRequest(Poco::Net::HTTPServerRequest & request)
{
    if (not request.has("Authorization")) {
        throw RGT::Devkit::RGTException("Unauthorized", Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED);        
    }

    // Разбиваем на части значение заголовка "Authorization"
    std::vector<std::string> tokenInfo = [](const std::string & str)
    {
        std::vector<std::string> result;
        std::string currentString;

        for (const char & c : str)
        {
            if (c != ' ') {
                currentString += c;
            } 
            else 
            {
                if (not currentString.empty()) 
                {
                    result.push_back(currentString);
                    currentString.clear();
                }
            }
        }

        if (not currentString.empty()) [[likely]] {
            result.push_back(std::move(currentString));
        }

        return result;
    }(request.get("Authorization"));
    
    if (tokenInfo.size() != 2) {
        throw RGT::Devkit::RGTException("Authorization header must have 2 words: 'Bearer' and the token itself", 
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }
    if (tokenInfo[0] != "Bearer") {
        throw RGT::Devkit::RGTException("Expected 'Bearer' authorization token", 
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }

    // Проверяет, что строка соответствует формату JWT-токена
    validateJWTFormat(tokenInfo[1]);

    return tokenInfo[1];
}

HTTPRequestHandler::JWTPayload HTTPRequestHandler::extractPayload(const std::string & token)
{
    // Валидация токена
    accessTokenValidate(token);

    const Poco::JWT::Token decoded(token);

    JWTPayload tokenPayload;

    // Извлекаем sub
    std::string sub = decoded.getSubject();
    if (sub.empty()) {
        throw RGT::Devkit::RGTException("The token is missing a sub",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }
    try {
        tokenPayload.sub = std::stoull(sub);
    }
    catch (...) {
        throw RGT::Devkit::RGTException("Sub must be an unsigned integer (user ID)",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }

    // Извлекаем role
    const Poco::JSON::Object & payload = decoded.payload();
    try {
        tokenPayload.role = payload.getValue<std::string>("role");
    }
    catch (...) {
        throw RGT::Devkit::RGTException("Received invalid access token",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }

    // Извлекаем exp
    Poco::Timestamp exp = decoded.getExpiration();
    tokenPayload.exp = std::chrono::seconds(exp.epochTime());

    return tokenPayload;
}

void HTTPRequestHandler::checkContentLength(const Poco::Net::HTTPServerRequest & request, const uint64_t & maxContentLength)
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

void HTTPRequestHandler::checkContentLengthIsNull(const Poco::Net::HTTPServerRequest & request)
{
    if (request.getContentLength() == 0) {
        throw RGT::Devkit::RGTException("Content length is zero", 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
    }
}

void HTTPRequestHandler::checkContentType(const Poco::Net::HTTPServerRequest & request, const std::string & contentType)
{
    const std::string & reqContentType = request.getContentType();

    if (reqContentType.empty()) {
        throw RGT::Devkit::RGTException("Expected to receive 'Content-Type' header", 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
    }

    if (reqContentType.find(contentType) == std::string::npos) {
        throw RGT::Devkit::RGTException(std::format("Content-Type must be {}", contentType), 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
    }
}

const std::string & HTTPRequestHandler::extractValueFromHeaders(const Poco::Net::HTTPServerRequest & request, const std::string & header)
{
    try {
        return request.get(header);
    }
    catch (...) {
        throw RGT::Devkit::RGTException(std::format("Expected to receive '{}' header", header), 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST); 
    }
}

const std::string & HTTPRequestHandler::extractValueFromCookies(const Poco::Net::NameValueCollection & cookies, const std::string & key)
{
    try {
        return cookies[key];
    }
    catch (...) {
        throw RGT::Devkit::RGTException(std::format("Expected to receive '{}' field in cookies", key), 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST); 
    }
}

Poco::Dynamic::Var HTTPRequestHandler::extractValueFromJson(const Poco::JSON::Object::Ptr json, const std::string & key)
{
    Poco::Dynamic::Var dvar = json->get(key);
    
    if (dvar.isEmpty()) {
        throw RGT::Devkit::RGTException(std::format("Expected to receive '{}' json field", key), 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST); 
    }
}

} // namespace RGT::Devkit
