#ifndef PATH_RESOLVER
#define PATH_RESOLVER

#include <cstdlib>
#include <string>
#include <vector>

#include "utils/string_utils.h"

class PathResolver {
public:
	PathResolver(const std::string& colon_delim_path):
		path_(utils::Split(colon_delim_path, ':')) {}

	PathResolver(): PathResolver(getenv("PATH")) {}

	// For each directory specified in the path, check if there is a file with
	// the given name and return the path to the first such file found.
	//
	// If there is a directory in the path that is invalid, then ignore it.
	// If no such file is found, return the same input string.
	std::string Resolve(const std::string& name) const;
private:
	std::vector<std::string> path_;
};

#endif  // PATH_RESOLVER