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
/*
 * /meta/1.0/index-system format:
 *
 * os;version;arch;type;time;dir
 * os;version;arch;type;time;dir
 * ........
 */
static char *line_get_value(const char *_Nonnull line, int index)
{
	/*
	 * Get the value of the index in the line.
	 * Warning: free() the return value after use.
	 */
	char *tmp = strdup(line);
	char *p = tmp;
	for (int i = 0; i < index; i++) {
		p = strchr(p, ';');
		if (p == NULL) {
			free(tmp);
			return NULL;
		}
		p++;
	}
	if (strchr(p, ';') == NULL) {
		*strchr(p, '\n') = '\0';
	} else {
		*strchr(p, ';') = '\0';
	}
	char *ret = strdup(p);
	free(tmp);
	return ret;
}
static const char *goto_next_line(const char *_Nonnull buf)
{
	/*
	 * Return the pointer to the next line.
	 */
	const char *p = strchr(buf, '\n');
	if (p == NULL) {
		return NULL;
	}
	return p + 1;
}
static char *get_current_line(const char *_Nonnull buf)
{
	/*
	 * Warning: free() the return value after use.
	 */
	char *tmp = strdup(buf);
	char *p = strchr(tmp, '\n');
	if (p == NULL) {
		free(tmp);
		return NULL;
	}
	*p = '\0';
	char *ret = strdup(tmp);
	free(tmp);
	return ret;
}
static char *get_lxc_index(const char *_Nullable mirror)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get lxc index info from mirror.
	 */
	char url[4096];
	if (mirror == NULL) {
		sprintf(url, "https://mirrors.bfsu.edu.cn/lxc-images/meta/1.0/index-system");
	} else {
		sprintf(url, "%s/meta/1.0/index-system", mirror);
	}
	const char *command[] = { "curl", "-L", "-s", url, NULL };
	return fork_execvp_get_stdout(command);
}
static char *lxc_get_image_dir(const char *_Nullable mirror, const char *_Nonnull os, const char *_Nonnull version, const char *_Nullable architecture, const char *_Nullable type)
{
	char *buf = get_lxc_index(mirror);
	const char *p = buf;
	if (p == NULL) {
		free(buf);
		return NULL;
	}
	if (architecture == NULL) {
		architecture = get_host_arch();
	}
	log("arch: %s\n", architecture);
	if (type == NULL) {
		type = "default";
	}
	log("p: %s\n", p);
	char *str_to_find = malloc(strlen(os) + strlen(version) + strlen(architecture) + strlen(type) + 10);
	sprintf(str_to_find, "%s;%s;%s;%s", os, version, architecture, type);
	log("str to find: %s\n", str_to_find);
	if (strstr(p, str_to_find) == NULL) {
		free(str_to_find);
		free(buf);
		return NULL;
	}
	p = strstr(p, str_to_find);
	p = strchr(p, '/');
	char *tmp = strdup(p);
	for (size_t i = 0; i < strlen(tmp); i++) {
		if (tmp[i] == '\n') {
			tmp[i] = '\0';
			break;
		}
	}
	char *ret = strdup(tmp);
	free(str_to_find);
	free(tmp);
	free(buf);
	return ret;
}
void lxc_get_image_list(const char *_Nullable mirror, const char *_Nullable architecture)
{
	if (architecture == NULL) {
		architecture = get_host_arch();
	}
	if (mirror == NULL) {
		mirror = "https://mirrors.bfsu.edu.cn/lxc-images";
	}
	char *buf = get_lxc_index(mirror);
	const char *p = buf;
	char *line = NULL;
	if (p == NULL) {
		free(buf);
		error("Failed to get index.");
	}
	bool found = false;
	while ((line = get_current_line(p)) != NULL) {
		char *os = line_get_value(line, 0);
		char *version = line_get_value(line, 1);
		char *arch = line_get_value(line, 2);
		char *type = line_get_value(line, 3);
		if (strcmp(arch, architecture) != 0) {
			free(os);
			free(version);
			free(arch);
			free(type);
			free(line);
			p = goto_next_line(p);
			continue;
		}
		found = true;
		cprintf("{yellow}%-13s {green}: {purple}%-10s {green}: {cyan}%-8s\n", os, version, type);
		free(os);
		free(version);
		free(arch);
		free(type);
		free(line);
		p = goto_next_line(p);
	}
	if (!found) {
		error("No image found.");
	}
	free(buf);
}
void lxc_search_image(const char *_Nullable mirror, const char *_Nonnull os, const char *_Nullable architecture)
{
	if (architecture == NULL) {
		architecture = get_host_arch();
	}
	if (mirror == NULL) {
		mirror = "https://mirrors.bfsu.edu.cn/lxc-images";
	}
	char *buf = get_lxc_index(mirror);
	const char *p = buf;
	char *line = NULL;
	if (p == NULL) {
		free(buf);
		error("Failed to get index.");
	}
	bool found = false;
	while ((line = get_current_line(p)) != NULL) {
		char *os_cur = line_get_value(line, 0);
		char *version = line_get_value(line, 1);
		char *arch = line_get_value(line, 2);
		char *type = line_get_value(line, 3);
		if (strcmp(arch, architecture) != 0 || strcmp(os_cur, os) != 0) {
			free(os_cur);
			free(version);
			free(arch);
			free(type);
			free(line);
			p = goto_next_line(p);
			continue;
		}
		found = true;
		cprintf("{yellow}%-13s {green}: {purple}%-10s {green}: {cyan}%-8s\n", os, version, type);
		free(os_cur);
		free(version);
		free(arch);
		free(type);
		free(line);
		p = goto_next_line(p);
	}
	free(buf);
	if (!found) {
		error("No image found.");
	}
}
void lxc_pull_image(const char *_Nullable mirror, const char *_Nonnull os, const char *_Nonnull version, const char *_Nullable architecture, const char *_Nullable type, const char *_Nonnull savedir)
{
	char *dir = lxc_get_image_dir(mirror, os, version, architecture, type);
	if (dir == NULL) {
		error("Image not found.");
	}
	if (mkdirs(savedir, 0755) == -1) {
		error("Failed to create directory.");
	}
	chdir(savedir);
	if (mirror == NULL) {
		mirror = "https://mirrors.bfsu.edu.cn/lxc-images";
	}
	char *url = malloc(strlen(mirror) + strlen(dir) + 114);
	sprintf(url, "%s/%srootfs.tar.xz", mirror, dir);
	cprintf("{base}Pulling {cyan}rootfs.tar.xz\n");
	const char *command[] = { "curl", "-L", "-s", url, "-o", "rootfs.tar.xz", NULL };
	fork_execvp(command);
	extract_archive("rootfs.tar.xz", ".");
	remove("rootfs.tar.xz");
	free(url);
	free(dir);
}
