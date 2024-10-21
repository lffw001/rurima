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
void get_input(char *_Nonnull message, char *_Nonnull buf)
{
	/*
	 * Warning: Not a safe function.
	 */
	cprintf("%s", message);
	fflush(stdout);
	fflush(stdin);
	scanf("%s", buf);
	fflush(stdout);
}
off_t get_file_size(const char *_Nonnull file)
{
	struct stat st;
	if (stat(file, &st) == -1) {
		return 0;
	}
	return st.st_size;
}
char *get_prefix(void)
{
	/*
	 * Warning: free() the return value after use.
	 */
	char *ret = getenv("PREFIX");
	if (ret == NULL) {
		ret = strdup("/");
	}
	return ret;
}
int mkdirs(const char *_Nonnull path, mode_t mode)
{
	/*
	 * A very simple implementation of mkdir -p.
	 * I don't know why it seems that there isn't an existing function to do this...
	 */
	remove(path);
	unlink(path);
	DIR *test = opendir(path);
	if (test != NULL) {
		closedir(test);
		return 0;
	}
	char buf[PATH_MAX];
	int ret = 0;
	/* If dir is path/to/mkdir
	 * We do:
	 * ret = mkdir("path",mode);
	 * ret = mkdir("path/to",mode);
	 * ret = mkdir("path/to/mkdir",mode);
	 * return ret;
	 */
	for (size_t i = 1; i < strlen(path); i++) {
		if (path[i] == '/') {
			for (size_t j = 0; j < i; j++) {
				buf[j] = path[j];
				buf[j + 1] = '\0';
			}
			ret = mkdir(buf, mode);
		}
	}
	// If the end of `dir` is not '/', create the last level of the directory.
	if (path[strlen(path) - 1] != '/') {
		ret = mkdir(path, mode);
	}
	return ret;
}
bool run_with_root(void)
{
	if (geteuid() == 0) {
		return true;
	} else {
		return false;
	}
}
char *get_host_arch(void)
{
	/*
	 * Get the cpu arch.
	 * We just need to check the macro we have.
	 *
	 * Do not free() the returned value.
	 */
	char *ret = NULL;
#if defined(__aarch64__)
	ret = "arm64";
#endif
#if defined(__alpha__)
	ret = "alpha";
#endif
#if defined(__arm__)
	ret = "arm";
#endif
#if defined(__armeb__)
	ret = "armeabi";
#endif
#if defined(__cris__)
	ret = "cris";
#endif
#if defined(__hexagon__)
	ret = "hexagon";
#endif
#if defined(__hppa__)
	ret = "hppa";
#endif
#if defined(__i386__)
	ret = "i386";
#endif
#if defined(__loongarch64__)
	ret = "loongarch64";
#endif
#if defined(__m68k__)
	ret = "m68k";
#endif
#if defined(__microblaze__)
	ret = "microblaze";
#endif
#if defined(__mips__)
	ret = "mips";
#endif
#if defined(__mips64__)
	ret = "mips64";
#endif
#if defined(__mips64el__)
	ret = "mips64el";
#endif
#if defined(__mipsel__)
	ret = "mipsel";
#endif
#if defined(__mipsn32__)
	ret = "mipsn32";
#endif
#if defined(__mipsn32el__)
	ret = "mipsn32el";
#endif
#if defined(__ppc__)
	ret = "ppc";
#endif
#if defined(__ppc64__)
	ret = "ppc64";
#endif
#if defined(__ppc64le__)
	ret = "ppc64le";
#endif
#if defined(__riscv32__)
	ret = "riscv32";
#endif
#if defined(__riscv64__)
	ret = "riscv64";
#endif
#if defined(__s390x__)
	ret = "s390x";
#endif
#if defined(__sh4__)
	ret = "sh4";
#endif
#if defined(__sh4eb__)
	ret = "sh4eb";
#endif
#if defined(__sparc__)
	ret = "sparc";
#endif
#if defined(__sparc32plus__)
	ret = "sparc32plus";
#endif
#if defined(__sparc64__)
	ret = "sparc64";
#endif
#if defined(__x86_64__)
	ret = "amd64";
#endif
#if defined(__xtensa__)
	ret = "xtensa";
#endif
#if defined(__xtensaeb__)
	ret = "xtensaeb";
#endif
	if (ret == NULL) {
		error("{red}Unknow cpu arch!\n");
	}
	return ret;
}