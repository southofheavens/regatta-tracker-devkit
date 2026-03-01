#ifndef __JWTPAYLOAD_H__
#define __JWTPAYLOAD_H__

#include <cstdint>
#include <string>
#include <chrono>

namespace RGT::Devkit
{

/// @brief Полезная нагрузка access токена
struct JWTPayload 
{
    uint64_t             sub;
    std::string          role;
    std::chrono::seconds exp;
};

} // namespace RGT::Devkit

#endif // __JWTPAYLOAD_H__
