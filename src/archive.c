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
static bool proot_exist(void)
{
	/*
	 * Test if proot exist.
	 * We use proot to execute ls, so that we can check if proot is really available.
	 */
	const char *cmd[] = { "proot", "ls", NULL };
	char *ret = fork_execvp_get_stdout(cmd);
	if (ret == NULL) {
		log("{red}proot not found.\n");
		return false;
	}
	free(ret);
	log("{green}proot found.\n");
	return true;
}
static bool proot_support_link2symlink(void)
{
	/*
	 * Test if proot support link2symlink.
	 * We use proot to execute ls, so that we can check if proot is really available.
	 */
	const char *cmd[] = { "proot", "--link2symlink", "ls", NULL };
	char *ret = fork_execvp_get_stdout(cmd);
	if (ret == NULL) {
		log("{red}proot not support --link2symlink.\n");
		return false;
	}
	free(ret);
	log("{green}proot support --link2symlink.\n");
	return true;
}
static char **get_extract_command(const char *_Nonnull file, const char *_Nonnull dir)
{
	/*
	 * Warning: free() after use.
	 * Only need to free **ret.
	 * ret[] does not need free().
	 * Get the command to extract the archive.
	 * Only support tar, gzip, xz.
	 * If the file is not supported, return NULL.
	 *
	 * If we are not running with root, and proot exist,
	 * we will use proot to extract the archive.
	 */
	char **ret = malloc(sizeof(char *) * 14);
	const char *file_command[] = { "file", "--brief", "--mime-type", file, NULL };
	char *type = fork_execvp_get_stdout(file_command);
	if (type == NULL) {
		error("{red}Failed to get file type!\n");
		free(ret);
		return NULL;
	}
	type[strlen(type) - 1] = '\0';
	if (!run_with_root() && proot_exist()) {
		if (proot_support_link2symlink()) {
			if (strcmp(type, "application/gzip") == 0) {
				ret[0] = "proot";
				ret[1] = "-0";
				ret[2] = "--link2symlink";
				ret[3] = "tar";
				ret[4] = "-xpzf";
				ret[5] = "-";
				ret[6] = "-C";
				ret[7] = (char *)dir;
				ret[8] = NULL;
			} else if (strcmp(type, "application/x-xz") == 0) {
				ret[0] = "proot";
				ret[1] = "-0";
				ret[2] = "--link2symlink";
				ret[3] = "tar";
				ret[4] = "-xpJf";
				ret[5] = "-";
				ret[6] = "-C";
				ret[7] = (char *)dir;
				ret[8] = NULL;
			} else if (strcmp(type, "application/x-tar") == 0) {
				ret[0] = "proot";
				ret[1] = "-0";
				ret[2] = "--link2symlink";
				ret[3] = "tar";
				ret[4] = "-xpf";
				ret[5] = "-";
				ret[6] = "-C";
				ret[7] = (char *)dir;
				ret[8] = NULL;
			} else {
				free(type);
				free(ret);
				return NULL;
			}
		} else {
			if (strcmp(type, "application/gzip") == 0) {
				ret[0] = "proot";
				ret[1] = "-0";
				ret[2] = "tar";
				ret[3] = "-xpzf";
				ret[4] = "-";
				ret[5] = "-C";
				ret[6] = (char *)dir;
				ret[7] = NULL;
			} else if (strcmp(type, "application/x-xz") == 0) {
				ret[0] = "proot";
				ret[1] = "-0";
				ret[2] = "tar";
				ret[3] = "-xpJf";
				ret[4] = "-";
				ret[5] = "-C";
				ret[6] = (char *)dir;
				ret[7] = NULL;
			} else if (strcmp(type, "application/x-tar") == 0) {
				ret[0] = "proot";
				ret[1] = "-0";
				ret[2] = "tar";
				ret[3] = "-xpf";
				ret[4] = "-";
				ret[5] = "-C";
				ret[6] = (char *)dir;
				ret[7] = NULL;
			} else {
				free(type);
				free(ret);
				return NULL;
			}
		}
	} else {
		if (!run_with_root()) {
			warning("{yellow}You are not running as root,");
			warning("{yellow}but proot not found, it might cause bugs unpacking rootfs.\n\n");
		}
		if (strcmp(type, "application/gzip") == 0) {
			ret[0] = "tar";
			ret[1] = "-xpzf";
			ret[2] = "-";
			ret[3] = "-C";
			ret[4] = (char *)dir;
			ret[5] = NULL;
		} else if (strcmp(type, "application/x-xz") == 0) {
			ret[0] = "tar";
			ret[1] = "-xpJf";
			ret[2] = "-";
			ret[3] = "-C";
			ret[4] = (char *)dir;
			ret[5] = NULL;
		} else if (strcmp(type, "application/x-tar") == 0) {
			ret[0] = "tar";
			ret[1] = "-xpf";
			ret[2] = "-";
			ret[3] = "-C";
			ret[4] = (char *)dir;
			ret[5] = NULL;
		} else {
			free(type);
			free(ret);
			return NULL;
		}
	}
	free(type);
	return ret;
}
static void show_progress(double per)
{
	/*
	 * Show progress bar.
	 */
	struct winsize size;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
	unsigned short width = size.ws_col - 10;
	unsigned short pos = (unsigned short)(width * per);
	printf("\033[?25l");
	printf("\r[\033[32m");
	for (unsigned short i = 0; i < pos; i++) {
		printf("\033[1;38;2;254;228;208m/");
	}
	for (unsigned short i = pos; i < width; i++) {
		printf("\033[0m ");
	}
	printf("\033[0m] %.2f%%", per * 100);
	fflush(stdout);
	printf("\033[?25h");
}
int extract_archive(const char *_Nonnull file, const char *_Nonnull dir)
{
	/*
	 * Extract the archive.
	 * The return value is not important,
	 * because we will error() directly if failed.
	 *
	 * We read the file, output it to stdout,
	 * and then pipe it to the extract command.
	 * So that we can show a progress bar by the size we output.
	 *
	 */
	check_dir_deny_list(dir);
	off_t size = get_file_size(file);
	if (size == 0) {
		error("{red}File size is 0!\n");
	}
	char **command = get_extract_command(file, dir);
	if (command == NULL) {
		error("{red}Unsupported file type!\n");
	}
	if (mkdirs(dir, 0755) == -1) {
		free(command);
		error("{red}Failed to create directory!\n");
	}
	cprintf("{base}Extracting {cyan}%s\n", file);
	FILE *fp = fopen(file, "rb");
	if (fp == NULL) {
		perror("fopen");
		free(command);
		error("{red}Failed to open file!\n");
	}
	int pipefd[2];
	if (pipe(pipefd) == -1) {
		perror("pipe");
		free(command);
		error("{red}Failed to create pipe!\n");
	}
	pid_t pid = fork();
	if (pid == 0) {
		close(pipefd[1]);
		dup2(pipefd[0], STDIN_FILENO);
		close(pipefd[0]);
		int fd = open("/dev/null", O_WRONLY);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		execvp(command[0], command);
		free(command);
		error("{red}Failed to exec command!\n");
	} else {
		close(pipefd[0]);
		// When buf is only 1024, it's very slow.
		// But when buf is 114514, it's fast enough.
		// So, this is the power of homo!!!!!!
		char *buf = malloc(114514);
		size_t bytes_read;
		size_t total_read = 0;
		while ((bytes_read = fread(buf, 1, 114514, fp)) > 0) {
			total_read += bytes_read;
			double progress = (double)total_read / (double)size;
			show_progress(progress);
			if (write(pipefd[1], buf, bytes_read) == -1) {
				perror("write");
				free(command);
				error("{red}Failed to write to stdout!");
			}
		}
		close(pipefd[1]);
		fclose(fp);
		wait(NULL);
		show_progress(1.0);
		printf("\n");
		free(command);
		free(buf);
		return 0;
	}
	free(command);
	return 0;
}
static int __tar_backup(const char *_Nonnull file, const char *_Nonnull dir)
{
	/*
	 * Backup dir as file(.tar format).
	 */
	mkdirs(file, 0755);
	rmdir(file);
	close(open(file, O_CLOEXEC | O_CREAT, 0755));
	char *file_realpath = realpath(file, NULL);
	char *dir_realpath = realpath(dir, NULL);
	int nullfd = open("/dev/null", O_RDWR);
	dup2(nullfd, STDOUT_FILENO);
	dup2(nullfd, STDERR_FILENO);
	int ret = 0;
	if (strstr(file_realpath, dir_realpath) != NULL) {
		chdir(dir);
		char exclude[PATH_MAX + 12] = { '\0' };
		sprintf(exclude, "--exclude=%s", file_realpath);
		char exclude2[PATH_MAX + 12] = { '\0' };
		sprintf(exclude2, "--exclude=%s", file);
		const char *command[] = { "tar", exclude, exclude2, "-cpf", file_realpath, ".", NULL };
		ret = fork_execvp(command);
	} else {
		chdir(dir);
		const char *command[] = { "tar", "-cpf", file_realpath, ".", NULL };
		ret = fork_execvp(command);
	}
	close(nullfd);
	free(file_realpath);
	free(dir_realpath);
	return ret;
}
static bool du_found(void)
{
	const char *command[] = { "du", "--version", NULL };
	char *ret = fork_execvp_get_stdout(command);
	if (ret == NULL) {
		log("{red}du not found.\n");
		return false;
	}
	free(ret);
	log("{green}du found.\n");
	return true;
}
int backup_dir(const char *_Nonnull file, const char *_Nonnull dir)
{
	/*
	 * Backup container as *.tar file.
	 * We compare the size of file and dir to show progress.
	 */
	struct stat st;
	if (stat(file, &st) == 0) {
		error("{red}File already exist!\n");
	}
	DIR *test = opendir(dir);
	if (test == NULL) {
		error("{red}Failed to open directory!\n");
	}
	closedir(test);
	if (!du_found()) {
		warning("{yellow}du not found, progress will not be shown.\n");
		int exstat = __tar_backup(file, dir);
		return exstat;
	}
	cprintf("{base}Getting total size to backup\n");
	off_t totalsize = get_dir_file_size(dir);
	cprintf("{base}Backing up to {cyan}%s\n", file);
	pid_t pid = fork();
	if (pid > 0) {
		int status;
		waitpid(pid, &status, WNOHANG);
		off_t totalsize_bk = totalsize;
		while (WIFEXITED(status) == 0) {
			off_t currentsize = get_file_size(file);
			totalsize = totalsize_bk;
			while (totalsize > FLT_MAX) {
				totalsize = totalsize / 1024;
				currentsize = currentsize / 1024;
				if (totalsize < FLT_MAX) {
					break;
				}
			}
			double progress = ((double)currentsize / (double)totalsize);
			show_progress(progress);
			usleep(100);
			waitpid(pid, &status, WNOHANG);
		}
		show_progress(1.0);
		return status;
	} else {
		int exstat = __tar_backup(file, dir);
		exit(exstat);
	}
	return 0;
}