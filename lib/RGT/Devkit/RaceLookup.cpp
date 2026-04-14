#include <RGT/Devkit/RaceLookup.h>
#include <RGT/Devkit/RGTException.h>

#include <Poco/Data/Session.h>
#include <Poco/Redis/Command.h>

namespace
{

std::optional<bool> isRaceExistsRedis(Poco::Redis::Client::Ptr redisClient, RGT::Devkit::RaceId raceId)
{
    Poco::Redis::BulkString redisReply = redisClient->execute<Poco::Redis::BulkString>
    (
        Poco::Redis::Command::get
        (
            std::format("is_race_exists:{}", RGT::Devkit::mapRaceIdToUint(raceId))
        )
    );
    if (not redisReply.isNull())
    {
        if (redisReply.value() == "0") {
            return false;
        }
        return true;
    }
    return std::nullopt;
}

std::optional<bool> isParticipationExistsRedis(Poco::Redis::Client::Ptr redisClient, 
    RGT::Devkit::RaceId raceId, RGT::Devkit::UserId userId)
{
    Poco::Redis::BulkString redisReply = redisClient->execute<Poco::Redis::BulkString>
    (
        Poco::Redis::Command::get
        (
            std::format
            (
                "p:r{}:u{}",
                RGT::Devkit::mapRaceIdToUint(raceId),
                RGT::Devkit::mapUserIdToUint(userId)
            )
        )
    );
    if (not redisReply.isNull())
    {
        if (redisReply.value() == "0") {
            return false;
        }
        return true;
    }
    return std::nullopt;
}

std::optional<RGT::Devkit::RaceStatus> getRaceStatusRedis(Poco::Redis::Client::Ptr redisClient, 
    RGT::Devkit::RaceId raceId)
{
    Poco::Redis::BulkString redisReply = redisClient->execute<Poco::Redis::BulkString>
    (
        Poco::Redis::Command::get
        (
            std::format("race_status:{}", RGT::Devkit::mapRaceIdToUint(raceId))
        )
    );
    if (not redisReply.isNull()) {
        return RGT::Devkit::mapStringToRaceStatus(redisReply.value());
    }
    return std::nullopt;
}

}

namespace RGT::Devkit
{

constexpr std::chrono::seconds default_ttl = std::chrono::seconds(900);

bool isRaceExists(Poco::Data::Session & session, Poco::Redis::PooledConnection & pc, RaceId raceId)
try
{
    // Сначала смотрим в Redis

    Poco::Redis::Client::Ptr redisClient = static_cast<Poco::Redis::Client::Ptr>(pc);
    if (redisClient == nullptr) 
    {
        // TODO лог
        throw std::exception{};
    }
    std::optional<bool> redisLookupResult = isRaceExistsRedis(redisClient, raceId);
    if (redisLookupResult.has_value()) {
        return *redisLookupResult;
    }

    // В Redis ничего не оказалось, поэтому ищем в БД и кэшируем в Redis

    bool exists = false;
    session <<
        "SELECT EXISTS ("
            "SELECT 1 "
            "FROM races "
            "WHERE id = $1"
        ");",
        Poco::Data::Keywords::bind(Devkit::mapRaceIdToUint(raceId)),
        Poco::Data::Keywords::into(exists),
        Poco::Data::Keywords::now;

    redisClient->execute<Poco::Redis::BulkString>
    (
        Poco::Redis::Command::set
        (
            std::format("is_race_exists:{}", Devkit::mapRaceIdToUint(raceId)),
            exists ? 1 : 0,
            true,
            Poco::Timespan(default_ttl.count(), 0)
        )
    );

    return exists;
}
catch (Poco::Exception e)
{
    std::cerr << "isRaceExists: " << e.displayText() << std::endl;
    throw;
}

bool isRaceExists(Poco::Data::SessionPool & sessionPool, Poco::Redis::PooledConnection & pc, RaceId raceId)
{ 
    Poco::Data::Session session = sessionPool.get();
    return isRaceExists(session, pc, raceId); 
}

bool isRaceExists(Poco::Data::SessionPool & sessionPool, RedisClientObjectPool & redisPool, RaceId raceId)
{ 
    Poco::Data::Session session = sessionPool.get();
    Poco::Redis::PooledConnection pc(redisPool, 500);
    return isRaceExists(session, pc, raceId);
}

bool isRaceExists(Poco::Data::Session & session, RedisClientObjectPool & redisPool, RaceId raceId)
{
    Poco::Redis::PooledConnection pc(redisPool, 500);
    return isRaceExists(session, pc, raceId);
}

bool isParticipationExists(Poco::Data::Session & session, Poco::Redis::PooledConnection & pc, 
    RaceId raceId, UserId userId)
try
{
    // Сначала смотрим в Redis

    Poco::Redis::Client::Ptr redisClient = static_cast<Poco::Redis::Client::Ptr>(pc);
    if (redisClient == nullptr) 
    {
        // TODO лог
        throw std::exception{};
    }
    std::optional<bool> redisLookupResult = isParticipationExistsRedis(redisClient, raceId, userId);
    if (redisLookupResult.has_value()) {
        return *redisLookupResult;
    }

    // В Redis ничего не оказалось, поэтому ищем в БД и кэшируем в Redis

    bool exists = false;
    session <<
        "SELECT EXISTS ("
            "SELECT 1 "
            "FROM participations "
            "WHERE race_id = $1 AND user_id = $2"
        ");",
        Poco::Data::Keywords::bind(RGT::Devkit::mapRaceIdToUint(raceId)),
        Poco::Data::Keywords::bind(RGT::Devkit::mapUserIdToUint(userId)),
        Poco::Data::Keywords::into(exists),
        Poco::Data::Keywords::now;

    redisClient->execute<Poco::Redis::BulkString>
    (
        Poco::Redis::Command::set
        (
            std::format
            (
                "p:r{}:u{}",
                RGT::Devkit::mapRaceIdToUint(raceId),
                RGT::Devkit::mapUserIdToUint(userId)
            ),
            exists ? 1 : 0,
            true,
            Poco::Timespan(default_ttl.count(), 0)
        )
    );

    return exists;
}
catch (Poco::Exception e)
{
    std::cerr << "isParticipationExists: " << e.displayText() << std::endl;
    throw;
}


bool isParticipationExists(Poco::Data::SessionPool & sessionPool, Poco::Redis::PooledConnection & pc, 
    RaceId raceId, UserId userId)
{
    Poco::Data::Session session = sessionPool.get();
    return isParticipationExists(session, pc, raceId, userId);
}

bool isParticipationExists(Poco::Data::SessionPool & sessionPool, RedisClientObjectPool & redisPool, 
    RaceId raceId, UserId userId)
{
    Poco::Data::Session session = sessionPool.get();
    Poco::Redis::PooledConnection pc(redisPool, 500);
    return isParticipationExists(session, pc, raceId, userId);
}

bool isParticipationExists(Poco::Data::Session & session, RedisClientObjectPool & redisPool, 
    RaceId raceId, UserId userId)
{
    Poco::Redis::PooledConnection pc(redisPool, 500);
    return isParticipationExists(session, pc, raceId, userId);
}

RaceStatus getRaceStatus(Poco::Data::Session & session, Poco::Redis::PooledConnection & pc, 
    RaceId raceId)
try
{
    // Сначала смотрим в Redis

    Poco::Redis::Client::Ptr redisClient = static_cast<Poco::Redis::Client::Ptr>(pc);
    if (redisClient == nullptr) 
    {
        // TODO лог
        throw std::exception{};
    }
    std::optional<RaceStatus> redisLookupResult = getRaceStatusRedis(redisClient, raceId);
    if (redisLookupResult.has_value()) {
        return *redisLookupResult;
    }

    // В Redis ничего не оказалось, поэтому ищем в БД и кэшируем в Redis

    std::string rawRaceStatus;
    session <<
        "SELECT status "
        "FROM races "
        "WHERE id = $1",
        Poco::Data::Keywords::bind(RGT::Devkit::mapRaceIdToUint(raceId)),
        Poco::Data::Keywords::into(rawRaceStatus),
        Poco::Data::Keywords::now;

    if (rawRaceStatus.empty()) {
        throw std::runtime_error(std::format("Race with id {} not exists", RGT::Devkit::mapRaceIdToUint(raceId)));
    }

    redisClient->execute<Poco::Redis::BulkString>
    (
        Poco::Redis::Command::set
        (
            std::format("race_status:{}", Devkit::mapRaceIdToUint(raceId)),
            rawRaceStatus,
            true,
            Poco::Timespan(default_ttl.count(), 0)
        )
    );

    return mapStringToRaceStatus(rawRaceStatus);
}
catch (Poco::Exception e)
{
    std::cerr << "getRaceStatus: " << e.displayText() << std::endl;
    throw;
}

RaceStatus getRaceStatus(Poco::Data::SessionPool & sessionPool, Poco::Redis::PooledConnection & pc,
    RaceId raceId)
{
    Poco::Data::Session session = sessionPool.get();
    return getRaceStatus(session, pc, raceId);
}

RaceStatus getRaceStatus(Poco::Data::SessionPool & sessionPool, RedisClientObjectPool & redisPool, 
    RaceId raceId)
{
    Poco::Data::Session session = sessionPool.get();
    Poco::Redis::PooledConnection pc(redisPool, 500);
    return getRaceStatus(session, pc, raceId);
}

RaceStatus getRaceStatus(Poco::Data::Session & session, RedisClientObjectPool & redisPool, 
    RaceId raceId)
{
    Poco::Redis::PooledConnection pc(redisPool, 500);
    return getRaceStatus(session, pc, raceId);
}

} // namespace RGT::Devkit
