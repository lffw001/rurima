
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
void *default_hook(const char *_Nonnull container_dir)
{
	char *hook_script = ""
			    "#!/bin/sh"
			    "# Fix DNS."
			    "rm /etc/resolv.conf"
			    "echo \"nameserver 1.1.1.1\" > /etc/resolv.conf"
			    "exit 0\n";
	if (gloal_config.hook_script == NULL) {
		char hook_script_path[PATH_MAX] = { '\0' };
		snprintf(hook_script_path, PATH_MAX, "%s/tmp/hook.sh", container_dir);
		chdir(container_dir);
		mkdir("tmp", 0666);
		int fd = open(hook_script_path, O_CREAT | O_WRONLY, 0777);
		write(fd, hook_script, strlen(hook_script));
		close(fd);
		if (run_with_root()) {
			char *argv[] = { "ruri", "-j", container_dir, "sh", "/tmp/hook.sh", NULL };
			fork_rexec(5, argv);
		} else {
			if (!rootless_supported()) {
				error("{red}Rootless mode is not supported on your system!\n");
			}
			char *argv[] = { "ruri", "-j", "-r", container_dir, "sh", "/tmp/hook.sh", NULL };
			fork_rexec(6, argv);
		}
	} else {
		char hook_script_path[PATH_MAX] = { '\0' };
		snprintf(hook_script_path, PATH_MAX, "%s/tmp/hook.sh", container_dir);
		chdir(container_dir);
		mkdir("tmp", 0666);
		int fd = open(hook_script_path, O_CREAT | O_WRONLY, 0777);
		int fd2 = open(gloal_config.hook_script, O_RDONLY);
		sendfile(fd, fd2, NULL, get_file_size(gloal_config.hook_script));
		close(fd);
		close(fd2);
		if (run_with_root()) {
			char *argv[] = { "ruri", "-j", container_dir, "sh", "/tmp/hook.sh", NULL };
			fork_rexec(5, argv);
		} else {
			if (!rootless_supported()) {
				error("{red}Rootless mode is not supported on your system!\n");
			}
			char *argv[] = { "ruri", "-j", "-r", container_dir, "sh", "/tmp/hook.sh", NULL };
			fork_rexec(6, argv);
		}
	}
}