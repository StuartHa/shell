#ifndef STRING_UTILS
#define STRING_UTILS

#include <string>
#include <vector>

namespace utils {

std::vector<std::string> Split(const std::string& str, char delim);

// Trim leading and trailing spaces from input string.
std::string Trim(const std::string& str);

}  // namespace utils

#endif  // STRING_UTILS
