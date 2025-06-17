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
static char *add_library_prefix(char *_Nonnull image)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Docker image need `library` prefix when no repository is specified.
	 * So we add it here.
	 */
	if (strchr(image, '/') != NULL) {
		return image;
	}
	char *ret = malloc(strlen(image) + 11);
	strcpy(ret, "library/");
	strcat(ret, image);
	// image is strdup()ed, so free it.
	free(image);
	return ret;
}
static void docker_pull_try_mirrors(const char *_Nonnull image, const char *_Nonnull tag, const char *_Nonnull architecture, const char *_Nonnull savedir, char *_Nonnull try_mirrorlist[], bool fallback)
{
	/*
	 * Try mirrors.
	 */
	char **rexec_argv = malloc(128 * sizeof(char *));
	rexec_argv[0] = NULL;
	for (int i = 0; try_mirrorlist[i] != NULL; i++) {
		cprintf("{base}Trying mirror: {cyan}%s\n", try_mirrorlist[i]);
		rurima_add_argv(&rexec_argv, "docker");
		rurima_add_argv(&rexec_argv, "pull");
		rurima_add_argv(&rexec_argv, "-i");
		rurima_add_argv(&rexec_argv, (char *)image);
		rurima_add_argv(&rexec_argv, "-t");
		rurima_add_argv(&rexec_argv, (char *)tag);
		rurima_add_argv(&rexec_argv, "-a");
		rurima_add_argv(&rexec_argv, (char *)architecture);
		rurima_add_argv(&rexec_argv, "-s");
		rurima_add_argv(&rexec_argv, (char *)savedir);
		rurima_add_argv(&rexec_argv, "-m");
		rurima_add_argv(&rexec_argv, (char *)try_mirrorlist[i]);
		if (fallback) {
			rurima_add_argv(&rexec_argv, "-f");
			if (rurima_fork_rexec(rexec_argv) == 0) {
				exit(0);
			} else {
				cprintf("{yellow}Mirror {cyan}%s {yellow}is not working!\n", try_mirrorlist[i]);
			}
		} else {
			if (rurima_fork_rexec(rexec_argv) == 0) {
				exit(0);
			} else {
				cprintf("{yellow}Mirror {cyan}%s {yellow}is not working!\n", try_mirrorlist[i]);
			}
		}
	}
	char *mirrorlist_builtin[] = { rurima_global_config.docker_mirror, "hub.xdark.top", "dockerpull.org", "hub.crdz.gq", "docker.1panel.live", "docker.unsee.tech", "docker.m.daocloud.io", "docker.kejinlion.pro", "registry.dockermirror.com", "hub.rat.dev", "dhub.kubesre.xyz", "docker.nastool.de", "docker.udayun.com", "docker.rainbond.cc", "hub.geekery.cn", "registry.hub.docker.com", NULL };
	for (int i = 0; mirrorlist_builtin[i] != NULL; i++) {
		cprintf("{base}Trying mirror: {cyan}%s\n", mirrorlist_builtin[i]);
		rexec_argv[0] = "docker";
		rexec_argv[1] = "pull";
		rexec_argv[2] = "-i";
		rexec_argv[3] = (char *)image;
		rexec_argv[4] = "-t";
		rexec_argv[5] = (char *)tag;
		rexec_argv[6] = "-a";
		rexec_argv[7] = (char *)architecture;
		rexec_argv[8] = "-s";
		rexec_argv[9] = (char *)savedir;
		rexec_argv[10] = "-m";
		rexec_argv[11] = (char *)mirrorlist_builtin[i];
		if (fallback) {
			rexec_argv[12] = "-f";
			rexec_argv[13] = NULL;
			if (rurima_fork_rexec(rexec_argv) == 0) {
				cprintf("\n{green}Success!\n");
				exit(0);
			} else {
				cprintf("\n{yellow}Mirror {cyan}%s {yellow}is not working!\n\n", mirrorlist_builtin[i]);
			}
		} else {
			rexec_argv[12] = NULL;
			if (rurima_fork_rexec(rexec_argv) == 0) {
				cprintf("\n{green}Success!\n");
				exit(0);
			} else {
				cprintf("\n{yellow}Mirror {cyan}%s {yellow}is not working!\n\n", mirrorlist_builtin[i]);
			}
		}
	}
	cprintf("{red}All mirrors are not working!\n");
}
/*
 * Subcommand for rurima.
 */
void rurima_docker(int argc, char **_Nonnull argv)
{
	if (!rurima_jq_exists()) {
		rurima_error("{red}jq is not installed!\n");
	}
	char *image = NULL;
	char *tag = NULL;
	char *architecture = NULL;
	char *savedir = NULL;
	char *page_size = NULL;
	char *mirror = NULL;
	char *runtime = NULL;
	bool quiet = false;
	bool fallback = false;
	bool try_mirrors = false;
	char *try_mirrorlist[1024] = { NULL };
	if (argc == 0) {
		rurima_error("{red}No subcommand specified!\n");
	}
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--image") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No image specified!\n");
			}
			image = strdup(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tag") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No tag specified!\n");
			}
			tag = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-process") == 0) {
			rurima_global_config.no_process = true;
		} else if (strcmp(argv[i], "-T") == 0 || strcmp(argv[i], "--try-mirrors") == 0) {
			try_mirrors = true;
			if (i + 1 < argc) {
				if (strchr(argv[i + 1], '-') != argv[i + 1]) {
					i++;
					for (int j = 0; j < 1024; j++) {
						if (try_mirrorlist[j] == NULL) {
							try_mirrorlist[j] = argv[i];
							try_mirrorlist[j + 1] = NULL;
							break;
						}
					}
				}
			}
		} else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--arch") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No architecture specified!\n");
			}
			architecture = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--runtime") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No container runtime specified!\n");
			}
			runtime = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--savedir") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No save directory specified!\n");
			}
			rurima_check_dir_deny_list(argv[i + 1]);
			rurima_mkdirs(argv[i + 1], 0755);
			savedir = realpath(argv[i + 1], NULL);
			if (savedir == NULL) {
				rurima_error("{red}Failed to create the save directory!\n");
			}
			i++;
		} else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--page_size") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No page size specified!\n");
			}
			page_size = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
			quiet = true;
			rurima_global_config.quiet = true;
		} else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fallback") == 0 || strcmp(argv[i], "--failback") == 0) {
			fallback = true;
		} else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mirror") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No mirror specified!\n");
			}
			mirror = argv[i + 1];
			i++;
		} else {
			rurima_error("{red}Unknown argument!\n");
		}
	}
	if (architecture == NULL) {
		architecture = rurima_docker_get_host_arch();
	}
	if (mirror == NULL) {
		mirror = rurima_global_config.docker_mirror;
	}
	if (strcmp(mirror, "registry-1.docker.io") != 0) {
		if (!quiet) {
			rurima_warning("{yellow}You are using unofficial mirror:{cyan} %s\n", mirror);
			rurima_warning("{yellow}You use it as your own risk.\n")
		}
	}
	if (strcmp(argv[0], "search") == 0) {
		if (image == NULL) {
			rurima_error("{red}No image specified!\n");
		}
		if (page_size == NULL) {
			page_size = "10";
		}
		rurima_docker_search(image, page_size, quiet, mirror);
	} else if (strcmp(argv[0], "tag") == 0) {
		if (page_size == NULL) {
			page_size = "10";
		}
		if (image == NULL) {
			rurima_error("{red}No image specified!\n");
		}
		image = add_library_prefix(image);
		rurima_docker_search_tag(image, page_size, architecture, quiet, mirror);
	} else if (strcmp(argv[0], "pull") == 0) {
		if (tag == NULL) {
			tag = "latest";
		}
		if (savedir == NULL) {
			rurima_error("{red}No save directory specified!\n");
		}
		if (image == NULL) {
			rurima_error("{red}No image specified!\n");
		}
		if (architecture == NULL) {
			architecture = rurima_docker_get_host_arch();
		}
		if (try_mirrors) {
			docker_pull_try_mirrors(image, tag, architecture, savedir, try_mirrorlist, fallback);
			exit(0);
		}
		if (!rurima_run_with_root()) {
			if (!proot_exist()) {
				rurima_warning("{yellow}You are not running with root, but proot not found, might cause bug unpacking rootfs!\n");
			}
		}
		image = add_library_prefix(image);
		struct RURIMA_DOCKER *config = rurima_docker_pull(image, tag, architecture, savedir, mirror, fallback);
		if (!quiet) {
			rurima_show_docker_config(config, savedir, runtime, quiet);
			if (config->architecture != NULL) {
				if (strcmp(config->architecture, architecture) != 0) {
					rurima_warning("{yellow}\nWarning: fallback mode detected!\n");
					rurima_warning("{yellow}The architecture of the image is not the same as the specified architecture!\n");
				}
			}
		}
		rurima_free_docker_config(config);
	} else if (strcmp(argv[0], "config") == 0) {
		if (tag == NULL) {
			tag = "latest";
		}
		if (image == NULL) {
			rurima_error("{red}No image specified!\n");
		}
		image = add_library_prefix(image);
		struct RURIMA_DOCKER *config = rurima_get_docker_config(image, tag, architecture, mirror, fallback);
		rurima_show_docker_config(config, savedir, runtime, quiet);
		if (!quiet) {
			if (config->architecture != NULL) {
				if (strcmp(config->architecture, architecture) != 0) {
					rurima_warning("{yellow}Warning: fallback mode detected!\n");
					rurima_warning("{yellow}The architecture of the image is not the same as the specified architecture!\n");
				}
			}
		}
		rurima_free_docker_config(config);
	} else if (strcmp(argv[0], "arch") == 0) {
		if (image == NULL) {
			rurima_error("{red}No image specified!\n");
		}
		image = add_library_prefix(image);
		if (tag == NULL) {
			rurima_error("{red}No tag specified!\n");
		}
		rurima_docker_search_arch(image, tag, mirror, fallback);
	} else if (strcmp(argv[0], "help") == 0 || strcmp(argv[0], "-h") == 0 || strcmp(argv[0], "--help") == 0) {
		cprintf("{base}Usage: rurima docker [subcommand] [options]\n");
		cprintf("{base}Subcommands:\n");
		cprintf("{base}  search: Search images from DockerHub.\n");
		cprintf("{base}  tag:    Search tags from DockerHub.\n");
		cprintf("{base}  pull:   Pull image from DockerHub.\n");
		cprintf("{base}  config: Get config of image from DockerHub.\n");
		cprintf("{base}  arch:   Search architecture of image from DockerHub.\n");
		cprintf("{base}  help:   Show help message.\n");
		cprintf("{base}Options:\n");
		cprintf("{base}  -i, --image: Image name.\n");
		cprintf("{base}  -t, --tag: Tag of image.\n");
		cprintf("{base}  -a, --arch: Architecture of image.\n");
		cprintf("{base}  -s, --savedir: Save directory of image.\n");
		cprintf("{base}  -p, --page_size: Page size of search.\n");
		cprintf("{base}  -m, --mirror: Mirror of DockerHub.\n");
		cprintf("{base}  -r, --runtime: runtime of container, support [ruri/proot/chroot].\n");
		cprintf("{base}  -q, --quiet: Quiet mode.\n");
		cprintf("{base}  -f, --fallback: Fallback mode.\n");
		cprintf("{base}  -T, --try-mirrors <mirror>: Try mirrors.\n");
		cprintf("\n{base}Note: please remove `https://` prefix from mirror url.\n");
		cprintf("{base}For example: `-m registry-1.docker.io`\n");
		cprintf("{base}You can add your perfered mirrors for `-T` option to try them first, for example: `-T hub.xdark.top -T dockerpull.org`\n");
	} else {
		rurima_error("{red}Invalid subcommand!\n");
	}
	free(image);
	free(savedir);
}
void rurima_lxc(int argc, char **_Nonnull argv)
{
	char *mirror = NULL;
	char *os = NULL;
	char *version = NULL;
	char *architecture = NULL;
	char *type = NULL;
	char *savedir = NULL;
	if (argc == 0) {
		rurima_error("{red}No subcommand specified!\n");
	}
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mirror") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No mirror specified!\n");
			}
			mirror = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-process") == 0) {
			rurima_global_config.no_process = true;
		} else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--os") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No os specified!\n");
			}
			os = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No version specified!\n");
			}
			version = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--arch") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No architecture specified!\n");
			}
			architecture = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--type") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No type specified!\n");
			}
			type = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--savedir") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No save directory specified!\n");
			}
			rurima_check_dir_deny_list(argv[i + 1]);
			savedir = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
			rurima_global_config.quiet = true;
		} else {
			rurima_error("{red}Unknown argument!\n");
		}
	}
	if (strcmp(argv[0], "pull") == 0) {
		if (os == NULL) {
			rurima_error("{red}No os specified!\n");
		}
		if (version == NULL) {
			rurima_error("{red}No version specified!\n");
		}
		if (savedir == NULL) {
			rurima_error("{red}No save directory specified!\n");
		}
		if (!rurima_run_with_root()) {
			if (!proot_exist()) {
				rurima_warning("{yellow}You are not running as root, but proot not found, might cause bug unpacking rootfs!\n");
			}
		}
		rurima_lxc_pull_image(mirror, os, version, architecture, type, savedir);
	} else if (strcmp(argv[0], "list") == 0) {
		rurima_lxc_get_image_list(mirror, architecture);
	} else if (strcmp(argv[0], "search") == 0) {
		if (os == NULL) {
			rurima_error("{red}No os specified!\n");
		}
		rurima_lxc_search_image(mirror, os, architecture);
	} else if (strcmp(argv[0], "arch") == 0) {
		if (os == NULL) {
			rurima_error("{red}No os specified!\n");
		}
		rurima_lxc_search_arch(mirror, os);
	} else if (strcmp(argv[0], "help") == 0 || strcmp(argv[0], "-h") == 0 || strcmp(argv[0], "--help") == 0) {
		cprintf("{base}Usage: rurima lxc [subcommand] [options]\n");
		cprintf("{base}Subcommands:\n");
		cprintf("{base}  pull: Pull image from LXC image server.\n");
		cprintf("{base}  list: List images from LXC image server.\n");
		cprintf("{base}  search: Search images from LXC image server.\n");
		cprintf("{base}  arch: Search architecture of images from LXC image server.\n");
		cprintf("{base}  help: Show help message.\n");
		cprintf("{base}Options:\n");
		cprintf("{base}  -m, --mirror: Mirror of LXC image server.\n");
		cprintf("{base}  -o, --os: OS of image.\n");
		cprintf("{base}  -v, --version: Version of image.\n");
		cprintf("{base}  -a, --arch: Architecture of image.\n");
		cprintf("{base}  -t, --type: Type of image.\n");
		cprintf("{base}  -s, --savedir: Save directory of image.\n");
		cprintf("\n{base}Note: please remove `https://` prefix from mirror url.\n");
		cprintf("{base}For example: `-m images.linuxcontainers.org`\n");
	} else {
		rurima_error("{red}Invalid subcommand!\n");
	}
}
void rurima_unpack(int argc, char **_Nonnull argv)
{
	char *file = NULL;
	char *dir = NULL;
	if (argc == 0) {
		rurima_error("{red}Unknown argument!\n");
	}
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No file specified!\n");
			}
			file = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dir") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No directory specified!\n");
			}
			rurima_check_dir_deny_list(argv[i + 1]);
			dir = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-process") == 0) {
			rurima_global_config.no_process = true;
		} else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			cprintf("{base}Usage: rurima unpack [options]\n");
			cprintf("{base}Options:\n");
			cprintf("{base}  -f, --file: File to unpack.\n");
			cprintf("{base}  -d, --dir: Directory to unpack.\n");
			cprintf("{base}  -h, --help: Show help message.\n");
			return;
		} else {
			rurima_error("{red}Unknown argument!\n");
		}
	}
	if (file == NULL) {
		rurima_error("{red}No file specified!\n");
	}
	if (dir == NULL) {
		rurima_error("{red}No directory specified!\n");
	}
	if (!rurima_run_with_root()) {
		rurima_warning("{yellow}You are not running as root, might cause bug unpacking rootfs!\n");
	}
	if (rurima_extract_archive(file, dir) != 0) {
		rurima_error("{red}Failed to extract archive!\n");
	}
}
void rurima_backup(int argc, char **_Nonnull argv)
{
	char *file = NULL;
	char *dir = NULL;
	if (argc == 0) {
		rurima_error("{red}Unknown argument!\n");
	}
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No file specified!\n");
			}
			file = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dir") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No directory specified!\n");
			}
			dir = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-process") == 0) {
			rurima_global_config.no_process = true;
		} else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			cprintf("{base}Usage: rurima backup [options]\n");
			cprintf("{base}Options:\n");
			cprintf("{base}  -f, --file: output file, tar format.\n");
			cprintf("{base}  -d, --dir: Directory to backup.\n");
			cprintf("{base}  -h, --help: Show help message.\n");
			return;
		} else {
			rurima_error("{red}Unknown argument!\n");
		}
	}
	if (file == NULL) {
		rurima_error("{red}No file specified!\n");
	}
	if (dir == NULL) {
		rurima_error("{red}No directory specified!\n");
	}
	if (rurima_backup_dir(file, dir) != 0) {
		rurima_warning("{yellow}tar exited with error status, but never mind, this might be fine\n");
	}
}
void rurima_pull(int argc, char **_Nonnull argv)
{
	if (argc == 0) {
		rurima_error("{red}Unknown argument!\n");
	}
	char *mirror = NULL;
	char *image = NULL;
	char *version = NULL;
	char *architecture = NULL;
	char *savedir = NULL;
	bool docker_only = false;
	bool fallback = false;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			cprintf("{base}Usage: rurima pull <options> [image]:[version] [savedir]\n");
			cprintf("{base}Options:\n");
			cprintf("{base}  -h, --help: Show help message.\n");
			cprintf("{base}  -m, --mirror: Mirror URL.\n");
			cprintf("{base}  -a, --arch: Architecture.\n");
			cprintf("{base}  -d, --docker: Only search dockerhub for image.\n");
			cprintf("{base}  -f, --fallback: Fallback mode, only for docker image.\n");
			cprintf("{base}Note: please remove `https://` prefix from mirror url.\n");
			cprintf("{base}This is just a wrap of docker and lxc subcommand.\n");
			cprintf("{base}It will re-exec itself to call the subcommand.\n");
			cprintf("{base}If -d option is not set, it will find lxc mirror first.\n");
			return;
		} else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mirror") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No mirror specified!\n");
			}
			mirror = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--arch") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No architecture specified!\n");
			}
			architecture = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--docker") == 0) {
			docker_only = true;
		} else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fallback") == 0) {
			fallback = true;
		} else {
			if (strstr(argv[i], ":") != NULL) {
				image = strtok(argv[i], ":");
				version = strtok(NULL, ":");
			} else {
				image = argv[i];
				version = "latest";
			}
			if (i + 1 < argc) {
				savedir = argv[i + 1];
			} else {
				rurima_error("{red}No save directory specified!\n");
			}
			char **rexec_argv = malloc(sizeof(char *) * 114);
			rexec_argv[0] = NULL;
			if (!docker_only && rurima_lxc_have_image(mirror, image, version, architecture, NULL)) {
				if (mirror == NULL) {
					mirror = rurima_global_config.lxc_mirror;
				}
				if (architecture == NULL) {
					architecture = rurima_lxc_get_host_arch();
				}
				rurima_add_argv(&rexec_argv, "lxc");
				rurima_add_argv(&rexec_argv, "pull");
				rurima_add_argv(&rexec_argv, "-m");
				rurima_add_argv(&rexec_argv, (char *)mirror);
				rurima_add_argv(&rexec_argv, "-o");
				rurima_add_argv(&rexec_argv, (char *)image);
				rurima_add_argv(&rexec_argv, "-v");
				rurima_add_argv(&rexec_argv, (char *)version);
				rurima_add_argv(&rexec_argv, "-a");
				rurima_add_argv(&rexec_argv, (char *)architecture);
				rurima_add_argv(&rexec_argv, "-s");
				rurima_add_argv(&rexec_argv, (char *)savedir);
				int exit_status = rurima_fork_rexec(rexec_argv);
				exit(exit_status);
			} else {
				if (mirror == NULL) {
					mirror = rurima_global_config.docker_mirror;
				}
				if (architecture == NULL) {
					architecture = rurima_docker_get_host_arch();
				}
				rurima_add_argv(&rexec_argv, "docker");
				rurima_add_argv(&rexec_argv, "pull");
				if (fallback) {
					rurima_add_argv(&rexec_argv, "-f");
				}
				rurima_add_argv(&rexec_argv, "-i");
				rurima_add_argv(&rexec_argv, (char *)image);
				rurima_add_argv(&rexec_argv, "-t");
				rurima_add_argv(&rexec_argv, (char *)version);
				rurima_add_argv(&rexec_argv, "-a");
				rurima_add_argv(&rexec_argv, (char *)architecture);
				rurima_add_argv(&rexec_argv, "-s");
				rurima_add_argv(&rexec_argv, (char *)savedir);
				rurima_add_argv(&rexec_argv, "-m");
				rurima_add_argv(&rexec_argv, (char *)mirror);
				int exit_status = rurima_fork_rexec(rexec_argv);
				exit(exit_status);
			}
		}
	}
	rurima_error("Emmmm, I think it will never reach here.\n");
}