#include "path_resolver.h"

#include <cstring>
#include <dirent.h>
#include <cerrno>
#include <iostream>

using std::cerr;
using std::endl;
using std::string;

string PathResolver::Resolve(const string& name) const {
	for (const string& dir : path_) {
		DIR* dir_stream = opendir(dir.c_str());
		if (dir_stream == nullptr) {
			continue;
		}
		for (struct dirent* dir_entry = readdir(dir_stream); dir_entry != nullptr;
			 dir_entry = readdir(dir_stream)) {
			if (name == string(dir_entry->d_name)) {
				return dir + (*dir.rbegin() == '/' ? "" : "/") + name;
			}
		}
	}
	// No matching file found.
	return name;
}
