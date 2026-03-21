#ifndef __S3_SUBSYSTEM_H__
#define __S3_SUBSYSTEM_H__

#include <Poco/Util/Subsystem.h>

#include <aws/core/auth/signer/AWSAuthV4Signer.h>
#include <aws/core/Aws.h>

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

namespace Aws 
{ 

namespace S3
{
    class S3Client; 
} // namespace Aws::S3

} // namespace Aws

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

    /// @brief Создаёт и инициализирует соединение с S3 хранилищем
    /// @return Указатель на S3Client
    /// @throw std::runtime_error при ошибке подключения
    std::unique_ptr<Aws::S3::S3Client> connectToS3
    (
        const std::string & accessKeyId,
        const std::string & secretKey,
        const std::string & endpointOverride,
        const std::string & region, 
        const Aws::Http::Scheme & scheme,
        bool verifySsl,
        bool useVirtualAddressing,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy signingPolicy = 
            Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never
    );

private:
    Aws::SDKOptions sdkOptions_;
    std::unique_ptr<Aws::S3::S3Client> s3Client_;
};

} // namespace RGT::Devkit::Subsystems

#endif // __S3_SUBSYSTEM_H__
