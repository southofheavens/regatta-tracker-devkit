#pragma once

#include <cstdint>
#include <string>
#include <chrono>

#include <RGT/Devkit/Types.h>

namespace RGT::Devkit
{

/// @brief Полезная нагрузка access токена
struct JWTPayload 
{
    UserId sub;
    UserRole role;
    std::chrono::seconds exp;
};

} // namespace RGT::Devkit
