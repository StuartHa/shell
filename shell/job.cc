#include "job.h"

#include <cstdio>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include "utils/string_utils.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using std::cerr;
using std::endl;
using std::string;
using std::vector;

void Command::Run(const PathResolver& path_resolver) const {
	string filename = path_resolver.Resolve(argv[0]);

	char* argv_arg[argv.size() + 1];
	for (size_t i = 0; i < argv.size(); i++) {
		argv_arg[i] = (char*)argv[i].c_str();
	}
	argv_arg[argv.size()] = NULL;

	char* envp_arg[1] = {NULL};

	execve(filename.c_str(), argv_arg, envp_arg);
}

// static
bool Command::Parse(const string command_str,
					Command* output_command) {
	vector<string> tokens = ::utils::Split(command_str, ' ');
	bool next_token_is_output = false;
	bool next_token_is_input = false;
	for (const string& token : tokens) {

		// Parse output file.
		if (next_token_is_output) {
			next_token_is_output = false;
			output_command->output = token;
		} else if (token == ">") {
			next_token_is_output = true;
		} else if (*token.begin() == '>') {
			output_command->output = token.substr(1);
		}
		// Parse input file.
		else if (next_token_is_input) {
			next_token_is_input = false;
			output_command->input = token;
		} else if (token == "<") {
			next_token_is_input = true;
		} else if (*token.begin() == '<') {
			output_command->input = token.substr(1);
		} else {
			// Parse normal argument.
			output_command->argv.push_back(token);
		}
	}
	if (output_command->argv.size() == 0) {
		return false;
	}
	if (next_token_is_output || next_token_is_input) {
		return false;
	}

	return true;
}

int Job::Run(const PathResolver& path_resolver, JobDescriptor* output_job_descriptor) const {
	vector<PipeDescriptor> pipe_descriptors;
	for (size_t i = 1; i < cmds.size(); i++) {
		int pipe_fd[2];
		int status = pipe(pipe_fd);
		if (status == -1) {
			cerr << "Error creating pipe: " << strerror(errno) << std::endl;
			return -1;
		}
		PipeDescriptor pipe_descriptor(pipe_fd[0], pipe_fd[1]);
		pipe_descriptors.push_back(pipe_descriptor);
	}

	output_job_descriptor->pgid = -1;
	for (size_t i = 0; i < cmds.size(); i++) {
		int pid = fork();
		if (pid == 0) {
			if (output_job_descriptor->pgid == -1) {
				int current_pid = getpid();
				setpgid(current_pid, current_pid);
				// Set PGID to PID of process group leader.
				output_job_descriptor->pgid = current_pid; // Unnecessary?
			} else {
				setpgid(getpid(), output_job_descriptor->pgid);
			}
			// Close file descriptors that aren't needed.
			for (size_t j = 1; j < cmds.size(); j++) {
				if (j != i + 1) {
					close(pipe_descriptors[j - 1].write_fd);
				}
				if (j != i) {
					close(pipe_descriptors[j - 1].read_fd);
				}
			}

			// Set up input file descriptor.
			if (cmds[i].input != "") {
				string input_filepath = cmds[i].input;
				int fd = open(input_filepath.c_str(), O_RDONLY);
				if (fd == -1) {
					cerr << "Error reading input file " << input_filepath << ": "
						 << strerror(errno) << std::endl;
					return -1;
				}
				int status = dup2(fd, fileno(stdin));
				if (status == -1) {
					cerr << "Error setting input: " << strerror(errno) << std::endl;
					return -1;
				}
			} else if (i > 0) {
				// No file input specified, use a pipe descriptor.
				int status = dup2(pipe_descriptors[i-1].read_fd, fileno(stdin));
				if (status == -1) {
					cerr << "Error setting input: " << strerror(errno) << std::endl;
					return -1;
				}
			}

			// Set up output file descriptor
			if (cmds[i].output != "") {
				string output_filepath = cmds[i].output;
				int fd = open(output_filepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
				if (fd == -1) {
					cerr << "Error writing output file " << output_filepath << ": "
						 << strerror(errno) << std::endl;
					return -1;
				}
				int status = dup2(fd, fileno(stdout));
				if (status == -1) {
					cerr << "Error setting output: " << strerror(errno) << std::endl;
					return -1;
				}
				cerr << "Set output to " << output_filepath << std::endl;
			} else if (i < cmds.size() - 1) {
				int status = dup2(pipe_descriptors[i].write_fd, fileno(stdout));
				if (status == -1) {
					cerr << "Error setting output: " << strerror(errno) << std::endl;
					return -1;
				}
			}

			cmds[i].Run(path_resolver);
			// Should never get here unless there was an error.
			cerr << "Error in running command " << cmds[i].argv[0] << ": "
				 <<	strerror(errno) << std::endl;
			return -1;
		} else {
			if (i == 0) {
				setpgid(pid, pid);
				output_job_descriptor->pgid = pid;
			}
			output_job_descriptor->pids.push_back(pid);
		}
	}

	for (const PipeDescriptor& pipe_descriptor : pipe_descriptors) {
		close(pipe_descriptor.read_fd);
		close(pipe_descriptor.write_fd);
	}

	return 0;
}

// static
bool Job::Parse(const string& job_str,
				Job* output_job) {
	string trimmed_jobs_str = ::utils::Trim(job_str);
	if (trimmed_jobs_str.back() == '&') {
		output_job->background = true;
		trimmed_jobs_str.pop_back();
	}
	vector<string> cmd_strs = ::utils::Split(trimmed_jobs_str, '|');
	for (size_t i = 0; i < cmd_strs.size(); i++) {
		Command cmd;
		if (!Command::Parse(cmd_strs[i], &cmd))
			return false;
		output_job->Pipe(cmd);
	}
	if (cmd_strs.size() == 0) {
		return false;
	}
	return true;
}