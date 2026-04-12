#pragma once

#include <cstdint>

namespace RGT::Devkit
{

enum class UserRole : uint8_t { Participant, Judge };
enum class RaceStatus : uint8_t { NotStarted, InProgress, Finished };
enum class UserId : uint64_t {};
enum class RaceId : uint64_t {};

} // namespace RGT::Devkit
