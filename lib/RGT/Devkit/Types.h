#pragma once

#include <cstdint>
#include <string>
#include <stdexcept>
#include <string_view>
#include <array>

namespace RGT::Devkit
{

enum class UserRole : uint8_t { Participant, Judge };

constexpr uint8_t number_of_user_roles = 2;

constexpr std::array<std::string_view, number_of_user_roles> userRoles = 
{
    std::string_view("participant"),
    std::string_view("judge")
};

[[nodiscard]] constexpr UserRole mapStringToUserRole(const std::string_view userRole)
{
    if (userRole == "participant") {
        return UserRole::Participant;
    }
    else if (userRole == "judge") {
        return UserRole::Judge;
    }
    throw std::runtime_error("mapStringToUserRole: unsupported string user role");
}

[[nodiscard]] constexpr std::string_view mapUserRoleToString(const UserRole userRole)
{
    switch (userRole)
    {
        case UserRole::Participant: 
            return userRoles[0];
        case UserRole::Judge:
            return userRoles[1];
        default:
            throw std::runtime_error("mapUserRoleToString: unsupported enum user role");
    }
}

/* - */

enum class RaceStatus : uint8_t { NotStarted, InProgress, Finished };

constexpr uint8_t number_of_race_statuses = 3;

constexpr std::array<std::string_view, number_of_race_statuses> raceStatuses = 
{
    std::string_view("not_started"),
    std::string_view("in_progress"),
    std::string_view("finished")
};

[[nodiscard]] constexpr RaceStatus mapStringToRaceStatus(const std::string_view raceStatus)
{
    if (raceStatus == "not_started") {
        return RaceStatus::NotStarted;
    }
    else if (raceStatus == "in_progress") {
        return RaceStatus::InProgress;
    }
    else if (raceStatus == "finished") {
        return RaceStatus::Finished;
    }
    throw std::runtime_error("mapStringToRaceStatus: unsupported string race status");
}

[[nodiscard]] constexpr std::string_view mapRaceStatusToString(const RaceStatus raceStatus)
{
    switch (raceStatus)
    {
        case RaceStatus::NotStarted:
            return raceStatuses[0];
        case RaceStatus::InProgress:
            return raceStatuses[1];
        case RaceStatus::Finished:
            return raceStatuses[2];
        default:
            throw std::runtime_error("mapRaceStatusToString: unsupported enum race status");
    }
}

/* - */

enum class UserId : uint64_t {};

[[nodiscard]] constexpr UserId mapUintToUserId(const uint64_t userId)
{ return static_cast<UserId>(userId); }

[[nodiscard]] constexpr uint64_t mapUserIdToUint(const UserId userId)
{ return static_cast<uint64_t>(userId); }

/* - */

enum class RaceId : uint64_t {};

[[nodiscard]] constexpr RaceId mapUintToRaceId(const uint64_t raceId)
{ return static_cast<RaceId>(raceId); }

[[nodiscard]] constexpr uint64_t mapRaceIdToUint(const RaceId raceId)
{ return static_cast<uint64_t>(raceId); }

} // namespace RGT::Devkit
