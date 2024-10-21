
// SPDX-License-Identifier: MIT
/*
 *
 * This file is part of rurima, with ABSOLUTELY NO WARRANTY.
 *
 * MIT License
 *
 * Copyright (c) 2024 Moe-hacker
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 */
#include "include/rurima.h"
int fork_execvp(const char *_Nonnull argv[])
{
	/*
	 * fork(2) and then execvp(3).
	 * Return the exit status of the child process.
	 */
	int pid = fork();
	if (pid == 0) {
		execvp(argv[0], (char **)argv);
		// If execvp(3) failed, exit as error status 114.
		exit(114);
	}
	int status = 0;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}
char *fork_execvp_get_stdout(const char *_Nonnull argv[])
{
	/*
	 * Warning: free() after use.
	 */
	// Create a pipe.
	int pipefd[2];
	if (pipe(pipefd) == -1) {
		perror("pipe");
		return NULL;
	}
	// fork(2) and then execvp(3).
	int pid = fork();
	if (pid == -1) {
		perror("fork");
		return NULL;
	}
	if (pid == 0) {
		// Close the read end of the pipe.
		close(pipefd[0]);
		// Redirect stdout and stderr to the write end of the pipe.
		dup2(pipefd[1], STDOUT_FILENO);
		dup2(pipefd[1], STDERR_FILENO);
		close(pipefd[1]);
		execvp(argv[0], (char **)argv);
		exit(114);
	} else {
		// Close the write end of the pipe.
		close(pipefd[1]);
		// Get the output from the read end of the pipe.
		size_t buffer_size = 1024;
		size_t total_read = 0;
		char *output = malloc(buffer_size);
		ssize_t bytes_read;
		while ((bytes_read = read(pipefd[0], output + total_read, buffer_size - total_read - 1)) > 0) {
			total_read += (size_t)bytes_read;
			if (total_read >= buffer_size - 1) {
				buffer_size *= 2;
				char *new_output = realloc(output, buffer_size);
				output = new_output;
			}
		}
		if (bytes_read == -1) {
			perror("read");
			free(output);
			close(pipefd[0]);
			return NULL;
		}
		output[total_read] = '\0';
		close(pipefd[0]);
		int status = 0;
		waitpid(pid, &status, 0);
		if (WEXITSTATUS(status) != 0) {
			free(output);
			return NULL;
		}
		return output;
	}
	return NULL;
}