#pragma once

#include <Poco/Util/Subsystem.h>
#include <Poco/Util/Application.h>
#include <Poco/Data/SessionPool.h>

#include <memory>

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

private:
    std::unique_ptr<Poco::Data::SessionPool> psqlPool_;
};

} // namespace RGT::Devkit::Subsystems
