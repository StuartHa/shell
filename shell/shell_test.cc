#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "shell.h"
#include "utils/test_utils.h"

using std::string;
using std::vector;

FILE* CreateOutputFile(const string& file_name) {
	const string dir_base_path = getenv("TEST_TMPDIR");
	const string output_file_path = dir_base_path +
		"/" + file_name;
	return fopen(output_file_path.c_str(), "w");
}

string ReadOutputFile(const string& output_file_path) {
	std::ifstream t(output_file_path);
	std::stringstream buffer;
	buffer << t.rdbuf();
	return buffer.str();

}

TEST(Execute, ls) {
	// Create a directory with a single empty file.
	CreateDirectoryWithFiles("test_ls", vector<string>{"a", "b"});
	string output_file_name = string(getenv("TEST_TMPDIR")) + "/ls_output.dat";

	Command cmd;
	string path_to_ls = string(getenv("TEST_TMPDIR")) + "/test_ls";
	cmd.argv = {"ls", path_to_ls.c_str()};
	cmd.output = output_file_name;

	Shell shell;
	shell.Execute(cmd);

	std::cerr << "Reading from " << output_file_name << std::endl;
	EXPECT_EQ(ReadOutputFile(output_file_name), "a\nb\n");
}

TEST(Execute, InputFile) {
	string output_file_name = string(getenv("TEST_TMPDIR")) + "/input_file_test_output.dat";

	// Create an input file.
	string input_file_name = string(getenv("TEST_TMPDIR")) +
		"/input_file.dat";
	std::ofstream output_stream (input_file_name, std::ofstream::out);
	output_stream << "This is just a test.";
	output_stream.close();

	Command cmd;
	cmd.argv = {"wc", "-w"};
	cmd.output = output_file_name;
	cmd.input = input_file_name;

	Shell shell;
	shell.Execute(cmd);

	EXPECT_EQ(ReadOutputFile(output_file_name), "5\n");
}

// Execute ls | wc -w. The "wc" command can take infinite input but "ls" has
// finite output, so this is a write limited pipe.
TEST(Execute, WriteLimitedPipe) {
	CreateDirectoryWithFiles("test_write_limited_pipe", vector<string>{"a", "b"});
	string output_file_name = string(getenv("TEST_TMPDIR")) + "/write_limited_pipe_output.dat";

	Command ls_cmd;
	string path_to_ls = string(getenv("TEST_TMPDIR")) + "/test_write_limited_pipe";
	ls_cmd.argv = {"ls", path_to_ls.c_str()};

	Command wc_cmd;
	// Count number of words.
	wc_cmd.argv = {"wc", "-w"};
	wc_cmd.output = output_file_name;

	Job piped_cmds = Job(ls_cmd).Pipe(wc_cmd);

	Shell shell;
	shell.Execute(piped_cmds);

	EXPECT_EQ(ReadOutputFile(output_file_name), "2\n");
}

// Execute yes | head -n 1. The "head" command only needs a single input but "yes" has
// infinite output, so this is a read limited pipe.
TEST(Execute, ReadLimitedPipe) {
	string output_file_name = string(getenv("TEST_TMPDIR")) + "/read_limited_pipe_output.dat";

	Command yes_cmd;
	yes_cmd.argv = {"yes"};

	Command head_cmd;
	// Count number of words.
	head_cmd.argv = {"head", "-n", "1"};
	head_cmd.output = output_file_name;

	Job piped_cmds = Job(yes_cmd).Pipe(head_cmd);

	Shell shell;
	shell.Execute(piped_cmds);

	EXPECT_EQ(ReadOutputFile(output_file_name), "y\n");
}

TEST(Execute, MultiplePipes) {
	// Create a directory with a single empty file.
	CreateDirectoryWithFiles("test_multiple_pipes", vector<string>{"a", "b"});
	string output_file_name = string(getenv("TEST_TMPDIR")) + "/multiple_pipes_output.dat";

	Command ls_cmd;
	string path_to_ls = string(getenv("TEST_TMPDIR")) + "/test_multiple_pipes";
	ls_cmd.argv = {"ls", path_to_ls.c_str()};

	Command wc_cmd_first;
	// Count number of words.
	wc_cmd_first.argv = {"wc", "-w"};

	Command wc_cmd_second;
	// Count number of words.
	wc_cmd_second.argv = {"wc", "-w"};
	wc_cmd_second.output = output_file_name;

	Job piped_cmds = Job(ls_cmd).Pipe(wc_cmd_first).Pipe(wc_cmd_second);

	Shell shell;
	shell.Execute(piped_cmds);

	EXPECT_EQ(ReadOutputFile(output_file_name), "1\n");
}

// Execute echo hi >ls_output.dat | wc -w >wc_output.dat
// Demonstrate that output file redirection works on a per-command basis.
TEST(Execute, PerCommandOutputFileRedirection) {
	string echo_output_file_name = string(getenv("TEST_TMPDIR")) + "/ls_output.dat";
	string wc_output_file_name = string(getenv("TEST_TMPDIR")) + "/wc_output.dat";

	Command echo_cmd;
	echo_cmd.argv = {"echo", "hi"};
	echo_cmd.output = echo_output_file_name;

	Command wc_cmd;
	// Count number of words.
	wc_cmd.argv = {"wc", "-w"};
	wc_cmd.output = wc_output_file_name;

	Job piped_cmds = Job(echo_cmd).Pipe(wc_cmd);

	Shell shell;
	shell.Execute(piped_cmds);

	EXPECT_EQ(ReadOutputFile(echo_output_file_name), "hi\n");
	EXPECT_EQ(ReadOutputFile(wc_output_file_name), "0\n");
}

// Execute cat <input_file.dat | cat >cat_output.txt
// Demonstrate that output file redirection works on a per-command basis.
TEST(Execute, PerCommandInputFileRedirection) {
	string cat_output_file_name = string(getenv("TEST_TMPDIR")) + "/cat_output.dat";

	// Create an input file.
	string input_file_name = string(getenv("TEST_TMPDIR")) +
		"/input_file.dat";
	std::ofstream output_stream (input_file_name, std::ofstream::out);
	output_stream << "This is just a test.";
	output_stream.close();

	Command cat_cmd_1;
	cat_cmd_1.argv = {"cat"};
	cat_cmd_1.input = input_file_name;

	Command cat_cmd_2;
	// Count number of words.
	cat_cmd_2.argv = {"cat"};
	cat_cmd_2.output = cat_output_file_name;

	Job piped_cmds = Job(cat_cmd_1).Pipe(cat_cmd_2);

	Shell shell;
	shell.Execute(piped_cmds);

	EXPECT_EQ(ReadOutputFile(cat_output_file_name), "This is just a test.");
}
