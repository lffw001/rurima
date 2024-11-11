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
struct RURIMA *init_config(void)
{
	/*
	 * Init an empty RURIMA struct.
	 */
	struct RURIMA *ret = malloc(sizeof(struct RURIMA));
	ret->rootfs_source = NULL;
	ret->rootfs_image = NULL;
	ret->rootfs_tag = NULL;
	ret->rootfs_arch = NULL;
	ret->rootfs_mirror = NULL;
	ret->rootfs_os = NULL;
	ret->rootfs_version = NULL;
	ret->rootfs_type = NULL;
	ret->rootfs_path = NULL;
	ret->host_dir = NULL;
	ret->hook_script = NULL;
	ret->hook_command[0] = NULL;
	ret->container.container_dir = NULL;
	ret->container.enable_seccomp = false;
	ret->container.no_new_privs = false;
	ret->container.no_warnings = false;
	ret->container.enable_unshare = false;
	ret->container.rootless = false;
	ret->container.mount_host_runtime = false;
	ret->container.command[0] = NULL;
	ret->container.env[0] = NULL;
	ret->container.extra_mountpoint[0] = NULL;
	ret->container.extra_ro_mountpoint[0] = NULL;
	ret->container.cross_arch = NULL;
	ret->container.qemu_path = NULL;
	ret->container.use_rurienv = true;
	ret->container.ro_root = false;
	ret->container.cpuset = NULL;
	ret->container.memory = NULL;
	ret->container.workdir = NULL;
	ret->container.just_chroot = false;
	return ret;
}
char *dump_ruri_config(struct RURIMA *_Nonnull config)
{
	/*
	 * Dump ruri config from RURIMA struct.
	 *
	 * Warning: free() the returned value after use.
	 */
	char *ret = NULL;
	// drop_caplist.
	char *drop_caplist[CAP_LAST_CAP + 1] = { NULL };
	int len = 0;
	for (int i = 0; true; i++) {
		if (config->container.drop_caplist[i] == INIT_VALUE) {
			len = i;
			break;
		}
		drop_caplist[i] = cap_to_name(config->container.drop_caplist[i]);
	}
	ret = k2v_add_comment(ret, "The capability to drop.");
	ret = k2v_add_config(char_array, ret, "drop_caplist", drop_caplist, len);
	// Make ASAN happy.
	for (int i = 0; i < len; i++) {
		cap_free(drop_caplist[i]);
	}
	// no_new_privs.
	ret = k2v_add_comment(ret, "Set NO_NEW_PRIVS bit.");
	ret = k2v_add_config(bool, ret, "no_new_privs", config->container.no_new_privs);
	// enable_unshare.
	ret = k2v_add_comment(ret, "Enable unshare feature.");
	ret = k2v_add_config(bool, ret, "enable_unshare", config->container.enable_unshare);
	// rootless.
	ret = k2v_add_comment(ret, "Run rootless container.");
	ret = k2v_add_config(bool, ret, "rootless", config->container.rootless);
	// mount_host_runtime.
	ret = k2v_add_comment(ret, "Mount runtime dirs from the host.");
	ret = k2v_add_config(bool, ret, "mount_host_runtime", config->container.mount_host_runtime);
	// ro_root.
	ret = k2v_add_comment(ret, "Make / read-only.");
	ret = k2v_add_config(bool, ret, "ro_root", config->container.ro_root);
	// no_warnings.
	ret = k2v_add_comment(ret, "Disable warnings.");
	ret = k2v_add_config(bool, ret, "no_warnings", config->container.no_warnings);
	// cross_arch.
	ret = k2v_add_comment(ret, "The arch for running cross-arch container.");
	ret = k2v_add_config(char, ret, "cross_arch", config->container.cross_arch);
	// qemu_path.
	ret = k2v_add_comment(ret, "The path of qemu-user static binary.");
	ret = k2v_add_config(char, ret, "qemu_path", config->container.qemu_path);
	// use_rurienv.
	ret = k2v_add_comment(ret, "If enable using .rurienv file.");
	ret = k2v_add_config(bool, ret, "use_rurienv", config->container.use_rurienv);
	// enable_seccomp.
	ret = k2v_add_comment(ret, "Enable built-in seccomp profile.");
	ret = k2v_add_config(bool, ret, "enable_seccomp", config->container.enable_seccomp);
	// cpuset.
	ret = k2v_add_comment(ret, "Cgroup cpuset limit.");
	ret = k2v_add_config(char, ret, "cpuset", config->container.cpuset);
	// memory.
	ret = k2v_add_comment(ret, "Cgroup memory limit.");
	ret = k2v_add_config(char, ret, "memory", config->container.memory);
	// workdir.
	ret = k2v_add_comment(ret, "The workdir.");
	ret = k2v_add_config(char, ret, "workdir", config->container.workdir);
	// just_chroot.
	ret = k2v_add_comment(ret, "Just chrrot.");
	ret = k2v_add_config(bool, ret, "just_chroot", config->container.just_chroot);
	// extra_mountpoint.
	for (int i = 0; true; i++) {
		if (config->container.extra_mountpoint[i] == NULL) {
			len = i;
			break;
		}
	}
	ret = k2v_add_comment(ret, "Extra mountpoint.");
	ret = k2v_add_config(char_array, ret, "extra_mountpoint", config->container.extra_mountpoint, len);
	// extra_ro_mountpoint.
	for (int i = 0; true; i++) {
		if (config->container.extra_ro_mountpoint[i] == NULL) {
			len = i;
			break;
		}
	}
	ret = k2v_add_comment(ret, "Extra read-only mountpoint.");
	ret = k2v_add_config(char_array, ret, "extra_ro_mountpoint", config->container.extra_ro_mountpoint, len);
	// env.
	for (int i = 0; true; i++) {
		if (config->container.env[i] == NULL) {
			len = i;
			break;
		}
	}
	ret = k2v_add_comment(ret, "Environment variable.");
	ret = k2v_add_config(char_array, ret, "env", config->container.env, len);
	// command.
	for (int i = 0; true; i++) {
		if (config->container.command[i] == NULL) {
			len = i;
			break;
		}
	}
	ret = k2v_add_comment(ret, "Default comand to run.");
	ret = k2v_add_config(char_array, ret, "command", config->container.command, len);
	// container_dir.
	ret = k2v_add_comment(ret, "The CONTAINER_DIR.");
	ret = k2v_add_config(char, ret, "container_dir", config->container.container_dir);
	return ret;
}