#include <RGT/Devkit/Connections/S3.h>

namespace RGT::Devkit::Connections
{

std::unique_ptr<S3::Client> connectToS3(const Poco::Util::AbstractConfiguration & cfg)
{
    return S3::makeClientFromEnv(cfg);
}

} // namespace RGT::Devkit::Connections
