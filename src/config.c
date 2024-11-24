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
	ruri_init_config(&ret->container);
	return ret;
}
char *dump_ruri_config(struct RURIMA *_Nonnull config)
{
	/*
	 * Dump ruri config from RURIMA struct.
	 *
	 * Warning: free() the returned value after use.
	 */
	char *ret = ruri_container_info_to_k2v(&config->container);
	return ret;
}