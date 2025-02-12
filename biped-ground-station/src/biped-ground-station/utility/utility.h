#ifndef UTILITY_H
#define UTILITY_H

#include <filesystem>

namespace biped
{
namespace ground_station
{
std::filesystem::path
appendHomePath(const std::filesystem::path& path);

std::filesystem::path
appendHomePath(const std::filesystem::path& path, const std::filesystem::path& path_home);

double
degreesToRadians(const double& degrees);

std::filesystem::path
expandHomePath(const std::filesystem::path& path);

std::filesystem::path
expandHomePath(const std::filesystem::path& path, const std::filesystem::path& path_home);

unsigned long
fpsToMilliseconds(const unsigned long& fps);

double
radiansToDegrees(const double& radians);

std::string
removeLeadingSeparator(const std::string& string, const char& separator);

std::string
removeTrailingSeparator(const std::string& string, const char& separator);
}
}

#endif // UTILITY_H
