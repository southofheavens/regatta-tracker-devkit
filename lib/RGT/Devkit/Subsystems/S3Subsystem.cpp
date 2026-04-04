#include <RGT/Devkit/Subsystems/S3Subsystem.h>
#include <RGT/Devkit/General.h>
#include <RGT/Devkit/Connections/S3.h>

#include <Poco/Util/Application.h>

namespace RGT::Devkit::Subsystems
{

const char * S3Subsystem::name() const
{ return "S3Subsystem"; }

Aws::S3::S3Client & S3Subsystem::getS3Client()
{ return *s3Client_; }
      
void S3Subsystem::initialize(Poco::Util::Application & app)
{  
    Aws::InitAPI(sdkOptions_);

    Poco::Util::LayeredConfiguration & cfg = app.config();

    s3Client_ = Connections::connectToS3(cfg);
}

void S3Subsystem::uninitialize()
{ Aws::ShutdownAPI(sdkOptions_); }

} // namespace RGT::Devkit::Subsystems
