#include "test_utils.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using std::string;
using std::vector;

void CreateDirectoryWithFiles(const string& dir_name,
							  const vector<string>& file_names) {
	const string dir_base_path = getenv("TEST_TMPDIR");
	const string dir_path = dir_base_path +
		(*dir_base_path.rbegin() == '/' ? "" : "/") + dir_name;
	mkdir(dir_path.c_str(), /*octal*/ 0777);

	for (const string& file_name : file_names) {
		string file_path = dir_path + "/" + file_name;
		// Create file by opening and closing it.
		int fd = open(file_path.c_str(), O_CREAT | O_EXCL);
		close(fd);
	}
}