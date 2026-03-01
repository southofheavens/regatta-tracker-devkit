#include <rgt/devkit/HTTPRequestHandler.h>

#include <Poco/JWT/Token.h>
#include <Poco/JWT/Serializer.h>
#include <Poco/JWT/Signer.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPServerRequest.h>

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
/// @note - У него корректная подпись
/// @note - Он не просрочен
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
        throw RGT::Devkit::RGTException("Received token have invalid signature",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }

    Poco::Timestamp expires = decoded.getExpiration();
    Poco::Timestamp now;
    
    if (expires < now) {
        throw RGT::Devkit::RGTException("Received token is expired or does not contain 'exp' field",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);    
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

RGT::Devkit::JWTPayload HTTPRequestHandler::extractPayload(const std::string & token)
{
    // На всякий случай проверим токен на соответствие формату JWT
    validateJWTFormat(token);
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
        throw RGT::Devkit::RGTException("Sub must contain a string that contains a unsigned integer (user ID)",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }

    // Извлекаем role
    const Poco::JSON::Object & payload = decoded.payload();
    try {
        tokenPayload.role = payload.getValue<std::string>("role");
    }
    catch (...) {
        throw RGT::Devkit::RGTException("Received token does not contain 'role' field or 'role' field is not string type",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }

    // Извлекаем exp
    Poco::Timestamp exp = decoded.getExpiration();
    tokenPayload.exp = std::chrono::seconds(exp.epochTime());

    return tokenPayload;
}

Poco::JSON::Object::Ptr HTTPRequestHandler::extractJsonObjectFromRequest(Poco::Net::HTTPServerRequest & req)
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

void HTTPRequestHandler::checkContentLength(Poco::Net::HTTPServerRequest & request, const uint64_t & maxContentLength)
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

void HTTPRequestHandler::checkContentLengthIsNull(Poco::Net::HTTPServerRequest & request)
{
    if (request.getContentLength() == 0) {
        throw RGT::Devkit::RGTException("Content length is zero", 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
    }
}

void HTTPRequestHandler::checkContentType(Poco::Net::HTTPServerRequest & request, const std::string & contentType)
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

const std::string & HTTPRequestHandler::extractValueFromHeaders(Poco::Net::HTTPServerRequest & request, const std::string & header)
{
    try {
        return request.get(header);
    }
    catch (...) {
        throw RGT::Devkit::RGTException(std::format("Expected to receive '{}' header", header), 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST); 
    }
}

const std::string & HTTPRequestHandler::extractValueFromCookies(Poco::Net::NameValueCollection & cookies, const std::string & key)
{
    try {
        return cookies[key];
    }
    catch (...) {
        throw RGT::Devkit::RGTException(std::format("Expected to receive '{}' field in cookies", key), 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST); 
    }
}

Poco::Dynamic::Var HTTPRequestHandler::extractValueFromJson(Poco::JSON::Object::Ptr json, const std::string & key)
{
    Poco::Dynamic::Var dvar = json->get(key);
    
    if (dvar.isEmpty()) {
        throw RGT::Devkit::RGTException(std::format("Expected to receive '{}' json field", key), 
            Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST); 
    }

    return dvar;
}

void HTTPRequestHandler::sendJsonResponse(Poco::Net::HTTPServerResponse & res, const std::string & status, 
    const std::string & message)
{
    Poco::JSON::Object json;
    json.set("status", status);
    json.set("message", message);

    std::ostream & out = res.send();
    json.stringify(out);
}

std::string HTTPRequestHandler::extractRefreshFromRequest(Poco::Net::HTTPServerRequest & request,
    std::shared_ptr<Poco::JSON::Object::Ptr> json)
{
    if (json == nullptr)
    {
        // TODO лог
        throw RGT::Devkit::RGTException("Internal server error", 
            Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
    }

    // Сначала пытаемся извлечь из cookies
    try 
    {
        Poco::Net::NameValueCollection cookies;
        request.getCookies(cookies); 
        return HTTPRequestHandler::extractValueFromCookies(cookies, "X-Refresh-token");
    }
    catch (...) 
    {
    }

    try {
        HTTPRequestHandler::checkContentType(request, "application/json");
    }
    catch (...) {
        throw RGT::Devkit::RGTException("Refresh token missing in cookies; content-type is not application/json",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }

    if (json->isNull()) 
    {
        try {
            *json = HTTPRequestHandler::extractJsonObjectFromRequest(request);
        }
        catch (...) 
        {
            throw RGT::Devkit::RGTException("Refresh token missing in cookies; request has invalid json or "
                "json array, not json object",
                Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        }
    }
        
    Poco::Dynamic::Var dvRefreshToken;
    try {
        dvRefreshToken = (*json)->get("refresh_token");
    }
    catch (...) {
        throw RGT::Devkit::RGTException("Expected to receive refresh token in the cookies/request body",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }

    try {
        return dvRefreshToken.extract<std::string>();
    }
    catch (...) {
        throw RGT::Devkit::RGTException("The refresh token must be represented as a string",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }
}

std::string HTTPRequestHandler::extractFingerprintFromRequest(Poco::Net::HTTPServerRequest & request,
    std::shared_ptr<Poco::JSON::Object::Ptr> json)
{
    if (json == nullptr)
    {
        // TODO лог
        throw RGT::Devkit::RGTException("Internal server error", 
            Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
    }

    // Сначала пытаемся извлечь из заголовков
    try {
        return HTTPRequestHandler::extractValueFromHeaders(request, "X-Fingerprint");
    }
    catch (...) 
    {
    }

    try {
        HTTPRequestHandler::checkContentType(request, "application/json");
    }
    catch (...) {
        throw RGT::Devkit::RGTException("Fingerprint missing in headers; content-type is not application/json",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }

    if (json->isNull()) 
    {
        try {
            *json = HTTPRequestHandler::extractJsonObjectFromRequest(request);
        }
        catch (...) 
        {
            throw RGT::Devkit::RGTException("Fingerprint missing in headers; request has invalid json or "
                "json array, not json object",
                Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        }
    }
    
    Poco::Dynamic::Var dvFingerprint;
    try {
        dvFingerprint = (*json)->get("fingerprint");
    }
    catch (...) {
        throw RGT::Devkit::RGTException("Expected to receive fingerprint in the headers/request body",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }

    try {
        return dvFingerprint.extract<std::string>();
    }
    catch (...) {
        throw RGT::Devkit::RGTException("The fingerprint must be represented as a string",
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }
}

} // namespace RGT::Devkit
