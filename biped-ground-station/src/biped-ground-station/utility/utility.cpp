#include <cmath>
#include <limits>

#include "utility/utility.h"

namespace biped
{
namespace ground_station
{
std::filesystem::path
appendHomePath(const std::filesystem::path& path)
{
    return appendHomePath(path, std::getenv("HOME"));
}

std::filesystem::path
appendHomePath(const std::filesystem::path& path, const std::filesystem::path& path_home)
{
    std::filesystem::path path_appended = path;

    if (path_appended.string()[0] != '/')
    {
        path_appended = path_home / path_appended;
    }

    return path_appended;
}


double
degreesToRadians(const double& degrees)
{
    return degrees / 180 * M_PI;
}

std::filesystem::path
expandHomePath(const std::filesystem::path& path)
{
    return expandHomePath(path, std::getenv("HOME"));
}

std::filesystem::path
expandHomePath(const std::filesystem::path& path, const std::filesystem::path& path_home)
{
    std::filesystem::path path_expanded = path;

    if (path_expanded.string().substr(0, 2) == "~/")
    {
        path_expanded = std::filesystem::path(path_expanded.string().erase(0, 2));
        path_expanded = path_home / path_expanded;
    }

    return path_expanded;
}

unsigned long
fpsToMilliseconds(const unsigned long& fps)
{
    if (fps == 0)
    {
        return std::numeric_limits<unsigned long>::max();
    }

    return (1.0 / fps) * 1000;
}

double
radiansToDegrees(const double& radians)
{
    return radians / M_PI * 180;
}

std::string
removeLeadingSeparator(const std::string& string, const char& separator)
{
    std::string string_processed = string;

    if (string_processed[0] == separator)
    {
        string_processed.erase(0, 1);
    }

    return string_processed;
}

std::string
removeTrailingSeparator(const std::string& string, const char& separator)
{
    std::string string_processed = string;

    if (string_processed[string_processed.size() - 1] == separator)
    {
        string_processed.erase(string_processed.size() - 1, 1);
    }

    return string_processed;
}
}
}
