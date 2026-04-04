#pragma once

#include <Poco/Util/Subsystem.h>

#include <aws/core/auth/signer/AWSAuthV4Signer.h>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>

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

class S3Subsystem : public Poco::Util::Subsystem
{
public:
    virtual const char * name() const final;

    Aws::S3::S3Client & getS3Client();

private:        
    virtual void initialize(Poco::Util::Application & app) final;

	virtual void uninitialize() final;

private:
    Aws::SDKOptions sdkOptions_;
    std::unique_ptr<Aws::S3::S3Client> s3Client_;
};

} // namespace RGT::Devkit::Subsystems
