#include <RGT/Devkit/Subsystems/PsqlSubsystem.h>
#include <RGT/Devkit/General.h>
#include <RGT/Devkit/Connections/PostgreSQL.h>

#include <Poco/Util/Application.h>
#include <Poco/Data/PostgreSQL/Connector.h>

namespace RGT::Devkit::Subsystems
{

const char * PsqlSubsystem::name() const
{ return "PsqlSubsystem"; }

Poco::Data::SessionPool & PsqlSubsystem::getPool()
{ return *psqlPool_; }
      
void PsqlSubsystem::initialize(Poco::Util::Application & app)
{  
    Poco::Data::PostgreSQL::Connector::registerConnector();

    Poco::Util::LayeredConfiguration & cfg = app.config();

    psqlPool_ = Connections::connectToPsql(cfg);
}

void PsqlSubsystem::uninitialize()
{ Poco::Data::PostgreSQL::Connector::unregisterConnector(); }

} // namespace RGT::Devkit::Subsystems
