#include <RGT/Devkit/Connections/PostgreSQL.h>
#include <RGT/Devkit/General.h>

#include <Poco/Util/AbstractConfiguration.h>

namespace RGT::Devkit::Connections
{

std::unique_ptr<Poco::Data::SessionPool> connectToPsql
(
    const std::string & host,
    const std::string & port,
    const std::string & dbname,
    const std::string & user, 
    const std::string & password,
    const uint8_t       minSessions,
    const uint8_t       maxSessions
)
{
    std::string connectionString = std::format
    (
        "host={0} port={1} dbname={2} user={3} password={4}", 
        host, port, dbname, user, password
    );

    std::unique_ptr<Poco::Data::SessionPool> sessionPool = std::make_unique<Poco::Data::SessionPool>("PostgreSQL", 
        connectionString, minSessions, maxSessions);

    try 
    {
        // Проверяем подключение к БД
        sessionPool->get() << "SELECT 1", Poco::Data::Keywords::now;
    }
    catch (...) 
    {
        throw std::runtime_error
        (
            std::format
            (
                "Connection attempt to postgresql failed with host {0} and port {1}",
                host, port
            )
        );
    }

    return sessionPool;   
} 

std::unique_ptr<Poco::Data::SessionPool> connectToPsql
(
    const Poco::Util::AbstractConfiguration & cfg
)
{
    std::optional<std::string> psqlHost = RGT::Devkit::getEnvOrCfg("PSQL_HOST", "psql.host", cfg);
    if (not psqlHost.has_value()) {
        throw std::runtime_error("The postgresql host is not set via environment variable (PSQL_HOST) or config (psql.host)");
    }
    std::optional<std::string> psqlPort = RGT::Devkit::getEnvOrCfg("PSQL_PORT", "psql.port", cfg);
    if (not psqlPort.has_value()) {
        throw std::runtime_error("The postgresql port is not set via environment variable (PSQL_PORT) or config (psql.port)");
    }
    std::optional<std::string> psqlUser = RGT::Devkit::getEnv("PSQL_USER");
    if (not psqlUser.has_value()) {
        throw std::runtime_error("The postgresql user is not set via environment variable (PSQL_USER)");
    }
    std::optional<std::string> psqlPassword = RGT::Devkit::getEnv("PSQL_PASSWORD");
    if (not psqlPassword.has_value()) {
        throw std::runtime_error("The postgresql password is not set via environment variable (PSQL_PASSWORD)");
    }

    return connectToPsql(psqlHost.value(), psqlPort.value(), cfg.getString("psql.dbname"), psqlUser.value(), 
        psqlPassword.value(), cfg.getUInt16("psql.min_sessions"), cfg.getUInt16("psql.max_sessions"));
}

} // namespace RGT::Devkit::Connections
