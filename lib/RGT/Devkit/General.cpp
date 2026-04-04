#include <RGT/Devkit/General.h>
#include <RGT/Devkit/RGTException.h>

#include <fstream>
#include <format>

#include <Poco/Util/AbstractConfiguration.h>

namespace
{

bool checkFormatOfEnvVarName(const std::string & envVarName)
{
    if (envVarName.empty()) {
        return false;
    }

    if (not std::isupper(envVarName[0])) {
        return false;
    }

    auto isLetterValid = [](char letter) -> bool
    {
        if (std::isupper(letter) or std::isdigit(letter) or letter == '_') {
            return true;
        }
        return false;
    };

    for (const char & letter : envVarName)
    {
        if (not isLetterValid(letter)) {
            return false;
        }
    }

    return true;
}

std::pair<std::string, std::string> parseEnvVar(const std::string & str)
{
    size_t equalSignPosition = str.find('=');

    if (equalSignPosition == std::string::npos) {
        throw std::runtime_error(std::format("Error parsing .env file: Missing equal sign {}", str));
    }

    std::string envVarName = str.substr(0, equalSignPosition);
    std::string envVarValue = str.substr(equalSignPosition + 1);

    if (not checkFormatOfEnvVarName(envVarName)) {
        throw std::runtime_error(std::format("Error parsing .env file: Invalid variable name {}", envVarName));
    }

    return { std::move(envVarName), std::move(envVarValue) };
}

void trimString(std::string & string)
{
    std::string::const_iterator firstNonSpaceChar = string.cbegin();
    while (firstNonSpaceChar != string.cend() and std::isspace(*firstNonSpaceChar)) {
        firstNonSpaceChar++;
    }

    if (firstNonSpaceChar == string.cend())
    {
        string.clear();
        return;
    }

    if (firstNonSpaceChar != string.cbegin()) {
        string.erase(string.cbegin(), firstNonSpaceChar);
    }

    auto firstNonSpaceCharFromEnd = string.crbegin();
    while (firstNonSpaceCharFromEnd != string.crend() and std::isspace(*firstNonSpaceCharFromEnd)) {
        firstNonSpaceCharFromEnd++;
    }

    if (firstNonSpaceCharFromEnd != string.crbegin()) {
        string.erase(firstNonSpaceCharFromEnd.base(), string.end());
    }
}

} // namespace 

namespace RGT::Devkit
{

std::string readLuaScript(const std::string & filename) 
{
    std::ifstream file(filename);
    
    if (not file.is_open()) {
        throw std::runtime_error("Cannot open Lua script: " + filename);
    }
    return std::string
    (
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

void readDotEnv()
{
    static bool alreadyRead = false;

    if (alreadyRead) {
        return;
    }

    std::fstream file;
    file.open(".env", std::ios::in);
    
    std::string line;
    while (std::getline(file, line))
    {
        trimString(line);
        if (line.empty()) {
            continue;
        }
        std::pair<std::string, std::string> envVar = parseEnvVar(line);
        if (setenv(envVar.first.c_str(), envVar.second.c_str(), 0) != 0) 
        {
            throw std::runtime_error
            (
                std::format
                (
                    "An error occurred while attempting to set an environment "
                    "variable with the name {} and the value {}",
                    envVar.first,
                    envVar.second
                )
            );
        }
    }

    alreadyRead = true;
}

std::optional<std::string> getEnv(const std::string & envVarName)
{
    char * envVar = getenv(envVarName.c_str());

    if (envVar == nullptr) {
        return std::nullopt;
    }
    return std::string(envVar);
}

std::optional<std::string> getEnvOrCfg(const std::string & envVarName, const std::string & cfgVarName,
    const Poco::Util::AbstractConfiguration & cfg)
{
    std::optional<std::string> env = getEnv(envVarName);

    if (env.has_value()) {
        return env;
    }

    try {
        return cfg.getString(cfgVarName);
    }
    catch (...) {
        return std::nullopt;
    }
}

} // namespace RGT::Devkit
