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
static void ubuntu(const char *_Nonnull dir, int argc, char **argv)
{
	char *rexec_argv[] = { "docker", "pull", "-i", "ubuntu", "-s", (char *)dir, NULL };
	if (fork_rexec(6, rexec_argv) != 0) {
		error("Failed to pull ubuntu image");
	}
	default_hook(dir);
}
static void debian(const char *_Nonnull dir, int argc, char **argv)
{
	char *rexec_argv[] = { "docker", "pull", "-i", "debian", "-s", (char *)dir, NULL };
	if (fork_rexec(6, rexec_argv) != 0) {
		error("Failed to pull debian image");
	}
	default_hook(dir);
}
static void centos(const char *_Nonnull dir, int argc, char **argv)
{
	char *rexec_argv[] = { "docker", "pull", "-i", "centos", "-s", (char *)dir, NULL };
	if (fork_rexec(6, rexec_argv) != 0) {
		error("Failed to pull centos image");
	}
	default_hook(dir);
}
static void fedora(const char *_Nonnull dir, int argc, char **argv)
{
	char *rexec_argv[] = { "docker", "pull", "-i", "fedora", "-s", (char *)dir, NULL };
	if (fork_rexec(6, rexec_argv) != 0) {
		error("Failed to pull fedora image");
	}
	default_hook(dir);
}
static void alpine(const char *_Nonnull dir, int argc, char **argv)
{
	char *rexec_argv[] = { "docker", "pull", "-i", "alpine", "-s", (char *)dir, NULL };
	if (fork_rexec(6, rexec_argv) != 0) {
		error("Failed to pull alpine image");
	}
	default_hook(dir);
}
static void almalinux(const char *_Nonnull dir, int argc, char **argv)
{
	char *rexec_argv[] = { "docker", "pull", "-i", "almalinux", "-s", (char *)dir, NULL };
	if (fork_rexec(6, rexec_argv) != 0) {
		error("Failed to pull almalinux image");
	}
	default_hook(dir);
}
static void busybox(const char *_Nonnull dir, int argc, char **argv)
{
	char *rexec_argv[] = { "docker", "pull", "-i", "busybox", "-s", (char *)dir, NULL };
	if (fork_rexec(6, rexec_argv) != 0) {
		error("Failed to pull busybox image");
	}
	default_hook(dir);
}
void template(const char *_Nonnull name, const char *_Nonnull dir, int argc, char **argv)
{
	if (strcmp(name, "ubuntu") == 0) {
		ubuntu(dir, argc, argv);
	} else if (strcmp(name, "debian") == 0) {
		debian(dir, argc, argv);
	} else if (strcmp(name, "centos") == 0) {
		centos(dir, argc, argv);
	} else if (strcmp(name, "fedora") == 0) {
		fedora(dir, argc, argv);
	} else if (strcmp(name, "almalinux") == 0) {
		almalinux(dir, argc, argv);
	} else if (strcmp(name, "alpine") == 0) {
		alpine(dir, argc, argv);
	} else if (strcmp(name, "busybox") == 0) {
		busybox(dir, argc, argv);
	} else {
		fprintf(stderr, "Unknown template: %s\n", name);
		exit(1);
	}
}
void list_template(void)
{
	char *tpl[] = { "ubuntu", "debian", "centos", "fedora", "almalinux", "alpine", "busybox", NULL };
	for (int i = 0; tpl[i] != NULL; i++) {
		printf("%s\n", tpl[i]);
	}
}