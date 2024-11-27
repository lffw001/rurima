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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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
// clang-format off
struct RURIMA_CONFIG gloal_config = {
	.docker_mirror = "registry-1.docker.io",
	.lxc_mirror = "images.linuxcontainers.org",
	.hook_script = NULL,
	.quiet = false,
};
// clang-format on
static void show_help(void)
{
	cprintf("{base}Usage: rurima [subcommand] [options]\n");
	cprintf("{base}Subcommands:\n");
	cprintf("{base}  docker: DockerHub support.\n");
	cprintf("{base}  lxc: LXC mirror support.\n");
	cprintf("{base}  unpack: Unpack rootfs.\n");
	cprintf("{base}  backup: Backup rootfs.\n");
	cprintf("{base}  ruri: Built-in ruri command.\n");
	cprintf("{base}  help: Show help message.\n");
	cprintf("{base}Options:\n");
	cprintf("{base}  -h, --help: Show help message.\n");
	cprintf("{base}  -v, --version: Show version info.\n");
	cprintf("{base}  -V, --version-code: Show version code.\n");
	cprintf("{base}See rurima [subcommand] help for further information.\n");
}
static void detect_suid_or_capability(void)
{
	struct stat st;
	if (stat("/proc/self/exe", &st) == 0) {
		if (((st.st_mode & S_ISUID) || (st.st_mode & S_ISGID)) && (geteuid() == 0 || getegid() == 0)) {
			error("{red}SUID or SGID bit detected on rurima, this is unsafe desu QwQ\n");
		}
	}
	cap_t caps = cap_get_file("/proc/self/exe");
	if (caps == NULL) {
		return;
	}
	char *caps_str = cap_to_text(caps, NULL);
	if (caps_str == NULL) {
		return;
	}
	if (strlen(caps_str) > 0) {
		error("{red}Capabilities detected on rurima, this is unsafe desu QwQ\n");
	}
	cap_free(caps);
	cap_free(caps_str);
}
int main(int argc, char **argv)
{
	detect_suid_or_capability();
	read_global_config();
#ifdef RURIMA_DEV
	warning("{red}You are using dev/debug build, if you think this is wrong, please rebuild rurima or get it from release page.\n");
#endif
	rurima_register_signal();
	check_dep();
	if (argc == 1) {
		show_help();
		return 0;
	}
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "docker") == 0 || strcmp(argv[i], "d") == 0) {
			if (i + 1 >= argc) {
				error("{red}No subcommand specified!\n");
			}
			docker(argc - i - 1, &argv[i + 1]);
			return 0;
		}
		if (strcmp(argv[i], "lxc") == 0 || strcmp(argv[i], "l") == 0) {
			if (i + 1 >= argc) {
				error("{red}No subcommand specified!\n");
			}
			lxc(argc - i - 1, &argv[i + 1]);
			return 0;
		}
		if (strcmp(argv[i], "backup") == 0 || strcmp(argv[i], "b") == 0) {
			backup(argc - i - 1, &argv[i + 1]);
			return 0;
		}
		if (strcmp(argv[i], "unpack") == 0 || strcmp(argv[i], "u") == 0) {
			unpack(argc - i - 1, &argv[i + 1]);
			return 0;
		}
		if (strcmp(argv[i], "help") == 0 || strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			show_help();
			return 0;
		}
		if (strcmp(argv[i], "version") == 0 || strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
			show_version_info();
			return 0;
		}
		if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version-code") == 0) {
			show_version_code();
			return 0;
		}
		if (strcmp(argv[i], "ruri") == 0 || strcmp(argv[i], "r") == 0) {
			unsetenv("LD_PRELOAD");
			pid_t pid = fork();
			if (pid == 0) {
				ruri(argc - i, &argv[i]);
			} else {
				int status;
				waitpid(pid, &status, 0);
				exit(WEXITSTATUS(status));
			}
			return 0;
		}
		show_help();
		error("{red}Invalid subcommand `%s`!\n", argv[i]);
		return 1;
	}
	return 0;
}
