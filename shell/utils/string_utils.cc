#include "string_utils.h"

using std::string;
using std::vector;

namespace utils {

vector<string> Split(const string& str, char delim) {
	vector<string> tokens;
	size_t start = 0;
	for (size_t end = 0; end < str.size(); end++) {
		if (str[end] == delim) {
			if (end > start) {
				tokens.push_back(str.substr(start, end - start));
			}
			start = end + 1;
		}
	}
	if (start < str.size()) {
		tokens.push_back(str.substr(start));
	}
	return tokens;
}

string Trim(const string& str) {
	string trimmed_str = str;

	// Erase leading spaces.
	while (trimmed_str.size() > 0 && trimmed_str.front() == ' ') {
		trimmed_str.erase(0, 1);
	}

	// Erase trailing spaces.
	while (trimmed_str.size() > 0 && trimmed_str.back() == ' ') {
		trimmed_str.erase(trimmed_str.size() - 1);
	}
	return trimmed_str;
}

}  // namespace utils
