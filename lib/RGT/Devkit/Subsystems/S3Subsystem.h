#pragma once

#include <Poco/Util/Subsystem.h>

#include <RGT/Devkit/S3/Client.h>

#include <memory>

namespace RGT::Devkit::Subsystems
{

class S3Subsystem : public Poco::Util::Subsystem
{
public:
    virtual const char * name() const final;

    S3::Client & getS3Client();

private:
    virtual void initialize(Poco::Util::Application & app) final;

    virtual void uninitialize() final;

private:
    std::unique_ptr<S3::Client> s3Client_;
};

} // namespace RGT::Devkit::Subsystems
