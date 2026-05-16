#include <RGT/Devkit/Subsystems/S3Subsystem.h>
#include <RGT/Devkit/Connections/S3.h>

#include <Poco/Util/Application.h>

namespace RGT::Devkit::Subsystems
{

const char * S3Subsystem::name() const
{ return "S3Subsystem"; }

S3::Client & S3Subsystem::getS3Client()
{ return *s3Client_; }

void S3Subsystem::initialize(Poco::Util::Application & app)
{
    s3Client_ = Connections::connectToS3(app.config());
}

void S3Subsystem::uninitialize()
{
    s3Client_.reset();
}

} // namespace RGT::Devkit::Subsystems
