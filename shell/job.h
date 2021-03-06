#ifndef PIPED_COMMANDS
#define PIPED_COMMANDS

#include <cstring>
#include <errno.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <sys/types.h>
#include <sys/wait.h>

#include "path_resolver.h"

struct Command {
	std::vector<std::string> argv;

	static bool Parse(const std::string command_str,
			   	   	  Command* output_command);
	void Run(const PathResolver& path_resolver) const;

	std::string input;
	std::string output;
};

// Forward declaration.
struct JobDescriptor;

struct Job {
	Job() {};

	Job(const Command& cmd): cmds{cmd} {}

	Job Pipe(const Command& cmd) {
		cmds.push_back(cmd);
		return *this;
	}
	int Run(const PathResolver& path_resolver, JobDescriptor* output_job_descriptor) const;

	// Parse a string into commands. A trailing '&' means the job should
	// run in the background.
	static bool Parse(const std::string& piped_commands_str,
	   	 	   	      Job* output_piped_commands);

	std::vector<Command> cmds;
	bool background = false;
	pid_t pgid;
private:
	struct PipeDescriptor {
		PipeDescriptor(int read_fd_arg, int write_fd_arg): read_fd(read_fd_arg),
			write_fd(write_fd_arg) {}

		int read_fd;
		int write_fd;
	};
};

struct JobDescriptor {
	pid_t pgid;
	std::vector<pid_t> pids;

	enum class Status {
		STOPPED,
		TERMINATED
	};

	int Wait(Status* output_status) {
		for (size_t i = 0; i < pids.size(); i++) {
			int status;
			// For child to change state.
			int pid = waitpid(pids[i], &status, WUNTRACED);
			if (WIFSTOPPED(status)) {
				*output_status = Status::STOPPED;
				std::cerr << "PID " << pid << " stopped." << std::endl;
			} else {
				*output_status = Status::TERMINATED;
				std::cerr << "PID " << pid << " terminated." << std::endl;
			}

			if (pid == -1) {
				std::cerr << "Error waiting for child: "
						  << strerror(errno) << std::endl;
				return -1;
			}
		}
		return 0; // Success
	}
};

#endif  // PIPED_COMMANDS
