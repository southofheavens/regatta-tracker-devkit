#pragma once

#include <Poco/Data/SessionPool.h>

namespace RGT::Devkit::TestTools
{

inline void deleteUser(Poco::Data::SessionPool & psqlPool, const uint64_t userId)
{
    Poco::Data::Session session = psqlPool.get();

    session << "DELETE FROM users WHERE id = $1",
        Poco::Data::Keywords::use(const_cast<uint64_t&>(userId)),
        Poco::Data::Keywords::now;
}

} // namespace RGT::Devkit::TestTools
