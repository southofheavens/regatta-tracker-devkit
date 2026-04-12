#pragma once

#include <cstdint>
#include <string>

namespace RGT::Devkit
{

enum class UserRole : uint8_t { Participant, Judge };

UserRole mapStringToUserRole(const std::string & userRole)
{
    if (userRole == "Participant") {
        return UserRole::Participant;
    }
    else if (userRole == "Judge") {
        return UserRole::Judge;
    }
    throw std::runtime_error("mapStringToUserRole: unsupported string user role");
}

std::string mapUserRoleToString(const UserRole userRole)
{
    switch (userRole)
    {
        case UserRole::Participant:
            return "Participant";
        case UserRole::Judge:
            return "Judge";
        default:
            throw std::runtime_error("mapUserRoleToString: unsupported enum user role");
    }
}

/* - */

enum class RaceStatus : uint8_t { NotStarted, InProgress, Finished };

RaceStatus mapStringToRaceStatus(const std::string & raceStatus)
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

std::string mapRaceStatusToString(const RaceStatus raceStatus)
{
    switch (raceStatus)
    {
        case RaceStatus::NotStarted:
            return "not_started";
        case RaceStatus::InProgress:
            return "in_progress";
        case RaceStatus::Finished:
            return "finished";
        default:
            throw std::runtime_error("mapRaceStatusToString: unsupported enum race status");
    }
}

/* - */

enum class UserId : uint64_t {};

UserId mapUintToUserId(const uint64_t userId)
{ return static_cast<UserId>(userId); }

uint64_t mapUserIdToUint(const UserId userId)
{ return static_cast<uint64_t>(userId); }

/* - */

enum class RaceId : uint64_t {};

RaceId mapUintToRaceId(const uint64_t raceId)
{ return static_cast<RaceId>(raceId); }

uint64_t mapRaceIdToUint(const RaceId raceId)
{ return static_cast<uint64_t>(raceId); }

} // namespace RGT::Devkit
