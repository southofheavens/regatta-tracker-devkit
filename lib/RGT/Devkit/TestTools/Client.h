#pragma once

#include <string>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/JSON/Object.h>
#include <Poco/StreamCopier.h>
#include <Poco/JSON/Parser.h>

#include <RGT/Devkit/TestTools/General.h>
#include <RGT/Devkit/TestTools/ConnectionRegistry.h>

#include <Poco/Data/Data.h>

namespace RGT::Devkit::TestTools
{

struct ExistingAccountTag {};

inline constexpr ExistingAccountTag existing_account{};

const std::string default_password    = "default_passworD1";
const std::string default_fingerprint = "default_fingerprint";
const std::string default_user_agent  = "default_user_agent";

enum class Role : uint8_t { Participant, Judge };

class User
{
public:
    User(const std::string & name, const std::string & surname, const std::string & login, const Role role)
        // : name_{name}
        // , surname_{surname}
        // , login_{login}
    {
        User::signUp(name, surname, login, role);
        User::login(login, default_password);
    }

    /// @brief Уже существующий пользователь: только login, без register/delete.
    User(ExistingAccountTag, const std::string & login, const std::string & password)
    {
        deleteOnDestroy_ = false;
        User::login(login, password);
        id_ = loadUserIdByLogin(login);
    }

    static User fromExisting(const std::string & login, const std::string & password)
    {
        return User(existing_account, login, password);
    }

    ~User()
    {
        if (deleteOnDestroy_) {
            RGT::Devkit::TestTools::deleteUser(ConnectionRegistry::instance().getPsqlPool(), id_);
        }
    }

    /// @brief Возвращает заготовку запроса с установленными заголовками X-Fingerprint и User-Agent, 
    /// где значения равны константам default_fingerprint и default_user_agent, и Authorization, который
    /// содержит access-токен
    Poco::Net::HTTPRequest getRequestBlank()
    {
        Poco::Net::HTTPRequest request;

        request.set("X-Fingerprint", default_fingerprint);
        request.set("User-Agent", default_user_agent);
        request.setCredentials("Bearer", accessToken_);

        return request;
    }

    /// @brief Обновляет access и refresh токены
    void refreshTokens()
    {
        Poco::Net::HTTPClientSession session("127.0.0.1", 80); 
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/auth/refresh");
            
        // Устанавливаем заголовки
        request.setContentType("application/json");
        request.set("X-Fingerprint", default_fingerprint);
        request.set("User-Agent", default_user_agent);

        // Формируем тело запроса
        Poco::JSON::Object::Ptr jsonBody(new Poco::JSON::Object);
        jsonBody->set("refresh_token", refreshToken_);

        // Приводим тело запроса из Poco::JSON::Object к std::string
        std::ostringstream bodyStream;
        jsonBody->stringify(bodyStream);
        std::string body = bodyStream.str();
        
        // Устанавливаем длину контента
        request.setContentLength(body.length());
        
        // Отправляем запрос
        std::ostream & os = session.sendRequest(request);
        os << body;
        
        // Получаем ответ
        Poco::Net::HTTPResponse response;
        std::istream & is = session.receiveResponse(response);
        if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK) {
            throw std::runtime_error("user could not get new access and refresh tokens pair");
        }

        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr result = parser.parse(is).extract<Poco::JSON::Object::Ptr>();

        Poco::Dynamic::Var dvAccess = result->get("access_token");
        Poco::Dynamic::Var dvRefresh = result->get("refresh_token");
        try 
        {
            accessToken_ = dvAccess.extract<std::string>();
            refreshToken_ = dvRefresh.extract<std::string>();
        }
        catch (const Poco::Exception & e) {
            throw std::runtime_error("cant extract tokens from response");
        }
    }

    uint64_t getId() const 
    { return id_; }

private:
    // Выполняется один раз в конструкторе
    void signUp(const std::string & name, const std::string & surname, const std::string & login, const Role role)
    {
        Poco::Net::HTTPClientSession session("127.0.0.1", 80); 
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/auth/register");
            
        // Устанавливаем заголовки
        request.setContentType("application/json");

        // Формируем тело запроса
        Poco::JSON::Object::Ptr jsonBody(new Poco::JSON::Object);
        jsonBody->set("name", name);
        jsonBody->set("surname", surname);
        jsonBody->set("login", login);
        jsonBody->set("password", default_password);
        jsonBody->set("role", (role == Role::Participant ? "participant" : "judge"));
        
        // Приводим тело запроса из Poco::JSON::Object к std::string
        std::ostringstream bodyStream;
        jsonBody->stringify(bodyStream);
        std::string body = bodyStream.str();
        
        // Устанавливаем длину контента
        request.setContentLength(body.length());
        
        // Отправляем запрос
        std::ostream & os = session.sendRequest(request);
        os << body;
        
        // Получаем ответ
        Poco::Net::HTTPResponse response;
        std::istream & is = session.receiveResponse(response);
        if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_CREATED) {
            throw std::runtime_error("user not created");
        }

        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr result = parser.parse(is).extract<Poco::JSON::Object::Ptr>();

        Poco::Dynamic::Var dvId = result->get("id");
        
        try {
            id_ = result->get("id").convert<uint64_t>();
        }
        catch (const Poco::Exception & e) {
            throw std::runtime_error("cant extract user id from response");
        }
    }

    uint64_t loadUserIdByLogin(const std::string & login)
    {
        Poco::Data::Session session = ConnectionRegistry::instance().getPsqlPool().get();
        uint64_t userId = 0;
        std::string loginCopy = login;
        session << "SELECT id FROM users WHERE login = $1",
            Poco::Data::Keywords::into(userId),
            Poco::Data::Keywords::use(loginCopy),
            Poco::Data::Keywords::now;
        if (userId == 0) {
            throw std::runtime_error("user id not found for login: " + login);
        }
        return userId;
    }

    // Выполняется один раз в конструкторе
    void login(const std::string & login, const std::string & password)
    {
        Poco::Net::HTTPClientSession session("127.0.0.1", 80); 
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/auth/login");
            
        // Устанавливаем заголовки
        request.setContentType("application/json");
        request.set("X-Fingerprint", default_fingerprint);
        request.set("User-Agent", default_user_agent);

        // Формируем тело запроса
        Poco::JSON::Object::Ptr jsonBody(new Poco::JSON::Object);
        jsonBody->set("login", login);
        jsonBody->set("password", password);

        // Приводим тело запроса из Poco::JSON::Object к std::string
        std::ostringstream bodyStream;
        jsonBody->stringify(bodyStream);
        std::string body = bodyStream.str();
        
        // Устанавливаем длину контента
        request.setContentLength(body.length());
        
        // Отправляем запрос
        std::ostream & os = session.sendRequest(request);
        os << body;
        
        // Получаем ответ
        Poco::Net::HTTPResponse response;
        std::istream & is = session.receiveResponse(response);
        if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK) {
            throw std::runtime_error("user could not login");
        }

        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr result = parser.parse(is).extract<Poco::JSON::Object::Ptr>();

        Poco::Dynamic::Var dvAccess = result->get("access_token");
        Poco::Dynamic::Var dvRefresh = result->get("refresh_token");
        try 
        {
            accessToken_ = dvAccess.extract<std::string>();
            refreshToken_ = dvRefresh.extract<std::string>();
        }
        catch (const Poco::Exception & e) {
            throw std::runtime_error("cant extract tokens from response");
        }
    }

    // const std::string & getName() const { return name_; }
    // void setName(const std::string & name) { name_ = name; }

    // const std::string & getSurname() const { return surname_; }
    // void setSurname(const std::string & surname) { surname_ = surname; }

    // const std::string & getLogin() const { return login_; }
    // void setLogin(const std::string & login) { login_ = login; }

    // const std::string & getAccessToken() const { return accessToken_; }
    // void setAccessToken(const std::string & token) { accessToken_ = token; }

    // const std::string & getRefreshToken() const { return refreshToken_; }
    // void setRefreshToken(const std::string & token) { refreshToken_ = token; }

private:
    bool deleteOnDestroy_{true};
    uint64_t id_{};
    // std::string name_;
    // std::string surname_;
    // std::string login_;

    std::string accessToken_;
    std::string refreshToken_;
};

// class Participant : public User
// {
// public:
//     Participant(const std::string & name, const std::string & surname, const std::string & login) 
//         : User(name, surname, login, "Participant")
//     {
//     }

//     virtual ~Participant() = default;

//     void upload(const std::string & time, double longitude, double latitude)
//     {

//     }
// };

// class Judge : public User
// {
// public:
//     Judge(const std::string & name, const std::string & surname, const std::string & login) 
//         : User(name, surname, login, "Judge")
//     {
//     }

//     virtual ~Judge() = default;

    
// };

} // namespace RGT::Devkit::TestTools
