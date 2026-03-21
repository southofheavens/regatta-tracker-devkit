#ifndef __PSQL_SUBSYSTEM_H__
#define __PSQL_SUBSYSTEM_H__

#include <Poco/Util/Subsystem.h>

#include <memory>

namespace Poco
{

namespace Util
{

class Application;

} // namespace Poco::Util

namespace Data
{

class SessionPool;

} // namespace Poco::Data

} // namespace Poco

namespace RGT::Devkit::Subsystems
{

class PsqlSubsystem : public Poco::Util::Subsystem
{
public:
    virtual const char * name() const final;

    Poco::Data::SessionPool & getPool();

private:        
    virtual void initialize(Poco::Util::Application & app) final;

	virtual void uninitialize() final;

    /// @brief Создаёт и инициализирует пул соединений с PostgreSQL
    /// @return Указатель на пул сессий
    /// @throw std::runtime_error при ошибке подключения
    std::unique_ptr<Poco::Data::SessionPool> connectToPsql
    (
        const std::string & host,
        const std::string & port,
        const std::string & dbname,
        const std::string & user, 
        const std::string & password,
        const uint8_t       minSessions,
        const uint8_t       maxSessions
    ); 

private:
    std::unique_ptr<Poco::Data::SessionPool> psqlPool_;
};

} // namespace RGT::Devkit::Subsystems

#endif // __PSQL_SUBSYSTEM_H__
