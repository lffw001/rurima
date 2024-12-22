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
void free_docker_config(struct DOCKER *_Nonnull config)
{
	/*
	 * free() the docker config.
	 */
	free(config->workdir);
	for (int i = 0; config->env[i] != NULL; i++) {
		free(config->env[i]);
	}
	for (int i = 0; config->command[i] != NULL; i++) {
		free(config->command[i]);
	}
	for (int i = 0; config->entrypoint[i] != NULL; i++) {
		free(config->entrypoint[i]);
	}
	free(config->architecture);
	free(config);
}
static void print_export_env(struct DOCKER *_Nonnull config)
{
	/*
	 * Print export env command.
	 */
	for (int i = 0; config->env[i] != NULL && config->env[i + 1] != NULL; i += 2) {
		printf("export %s=\"%s\"\n", config->env[i], config->env[i + 1]);
	}
}
static void print_chroot_command(struct DOCKER *_Nonnull config, char *_Nullable savedir)
{
	/*
	 * Print command to use chroot as runtime.
	 */
	print_export_env(config);
	printf("mount --bind %s %s\n", savedir == NULL ? "/path/to/container" : savedir, savedir == NULL ? "/path/to/container" : savedir);
	printf("mount --bind /dev ");
	printf("%s/dev\n", savedir == NULL ? "/path/to/container" : savedir);
	printf("mount --bind /proc ");
	printf("%s/proc\n", savedir == NULL ? "/path/to/container" : savedir);
	printf("mount --bind /sys ");
	printf("%s/sys\n", savedir == NULL ? "/path/to/container" : savedir);
	printf("chroot ");
	printf("%s ", savedir == NULL ? "/path/to/container" : savedir);
	if (config->command[0] != NULL) {
		for (int i = 0; config->command[i] != NULL; i++) {
			printf("%s ", config->command[i]);
		}
	} else if (config->entrypoint[0] != NULL) {
		if (config->workdir != NULL) {
			printf("%s/", config->workdir);
		}
		for (int i = 0; config->entrypoint[i] != NULL; i++) {
			printf("%s ", config->entrypoint[i]);
		}
	}
	printf("\n");
}
static void print_proot_command(struct DOCKER *_Nonnull config, char *_Nullable savedir)
{
	/*
	 * Print command to use proot as runtime.
	 */
	print_export_env(config);
	printf("OTHER_ARGS=\"\"\n");
	printf("QEMU_PATH=\"\"\n");
	printf("proot $OTHER_ARGS -0 ");
	if (config->workdir != NULL) {
		printf("-w %s ", config->workdir);
	}
	if (strcmp(config->architecture, docker_get_host_arch()) != 0) {
		printf("-q $QEMU_PATH ");
	}
	printf("-r %s ", savedir == NULL ? "/path/to/container" : savedir);
	if (config->command[0] != NULL) {
		for (int i = 0; config->command[i] != NULL; i++) {
			printf("%s ", config->command[i]);
		}
	} else if (config->entrypoint[0] != NULL) {
		for (int i = 0; config->entrypoint[i] != NULL; i++) {
			printf("%s ", config->entrypoint[i]);
		}
	}
	printf("\n");
}
static void print_ruri_command(struct DOCKER *_Nonnull config, char *_Nullable savedir)
{
	/*
	 * Print command to use ruri as runtime.
	 */
	printf("rurima r ");
	printf("-w ");
	if (config->workdir != NULL) {
		printf("-W %s ", config->workdir);
	}
	if (strcmp(config->architecture, docker_get_host_arch()) != 0) {
		printf("-a %s ", config->architecture);
		printf("-q /path/to/qemu-%s-static ", config->architecture);
	}
	for (int i = 0; config->env[i] != NULL && config->env[i + 1] != NULL; i += 2) {
		printf("-e \"%s\" ", config->env[i]);
		if (strcmp(config->env[i + 1], "") != 0) {
			printf("\"%s\" ", config->env[i + 1]);
		} else {
			printf("\" \" ");
		}
	}
	printf("%s ", savedir == NULL ? "/path/to/container" : savedir);
	if (config->command[0] != NULL) {
		for (int i = 0; config->command[i] != NULL; i++) {
			printf("%s ", config->command[i]);
		}
	} else if (config->entrypoint[0] != NULL) {
		for (int i = 0; config->entrypoint[i] != NULL; i++) {
			printf("%s ", config->entrypoint[i]);
		}
	}
	printf("\n");
}
void show_docker_config(struct DOCKER *_Nonnull config, char *_Nullable savedir, char *_Nullable runtime, bool quiet)
{
	/*
	 * Show docker config.
	 */
	if (!quiet) {
		cprintf("{base}\nConfig:\n");
		cprintf("{base}  Workdir:\n    {cyan}%s\n", config->workdir == NULL ? "NULL" : config->workdir);
		cprintf("{base}  Env:\n");
		if (config->env[0] == NULL) {
			cprintf("{cyan}NULL\n");
		} else {
			for (int i = 0; config->env[i] != NULL; i += 2) {
				cprintf("{cyan}    %s = ", config->env[i]);
				cprintf("{cyan}%s\n", config->env[i + 1]);
			}
		}
		cprintf("{base}  Command:\n    ");
		if (config->command[0] == NULL) {
			cprintf("{cyan}NULL\n");
		} else {
			for (int i = 0; config->command[i] != NULL; i++) {
				cprintf("{cyan}%s ", config->command[i]);
			}
			cprintf("{clear}\n");
		}
		cprintf("{base}  Entrypoint:\n    ");
		if (config->entrypoint[0] == NULL) {
			cprintf("{cyan}NULL\n");
		} else {
			for (int i = 0; config->entrypoint[i] != NULL; i++) {
				cprintf("{cyan}%s ", config->entrypoint[i]);
			}
			cprintf("{clear}\n");
		}
	}
	if (runtime == NULL) {
		runtime = "ruri";
	}
	if (strcmp(runtime, "ruri") == 0) {
		if (!quiet) {
			cprintf("{base}Run with ruri:\n");
			printf("\033[38;2;219;240;240m\n");
		}
		print_ruri_command(config, savedir);
	} else if (strcmp(runtime, "proot") == 0) {
		if (!quiet) {
			cprintf("{base}Run with proot:\n");
			printf("\033[38;2;219;240;240m\n");
		}
		print_proot_command(config, savedir);
		if (!quiet) {
			warning("\n{yellow}Please replace [OTHER_ARGS] with your proot args!");
		}
	} else if (strcmp(runtime, "chroot") == 0) {
		if (!quiet) {
			cprintf("{base}Run with chroot:\n");
			printf("\033[38;2;219;240;240m\n");
		}
		print_chroot_command(config, savedir);
		if (!quiet) {
			if (strcmp(config->architecture, docker_get_host_arch()) != 0) {
				warning("{yellow}For chroot, please configure binfimt_misc manually!\n");
			}
		}
	} else {
		error("Unknown container runtime!");
	}
	if (!quiet) {
		printf("\n\033[0m");
		if (savedir == NULL) {
			warning("{yellow}Please replace /path/to/container with your container path!\n");
		}
		if (strcmp(config->architecture, docker_get_host_arch()) != 0) {
			warning("{yellow}Please replace /path/to/qemu-%s-static with your qemu binary path!\n", config->architecture);
		}
	}
}
static char *get_auth_server_from_header(const char *_Nonnull header, bool fallback)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get auth server from header.
	 * Example:
	 * www-authenticate: Bearer realm="https://auth.docker.io/token",service="registry.docker.io"
	 *
	 */
	// Just to show you how ugly if we don't force lowercase the header.
	const char *p = strstr_ignore_case(header, "wWw-aUthEntIcAtE: ");
	if (p == NULL) {
		if (fallback) {
			return NULL;
		}
		error("{red}No auth server found!\n");
	}
	p = strstr(p, "realm=");
	if (p == NULL) {
		if (fallback) {
			return NULL;
		}
		error("{red}No auth server found!\n");
	}
	p = strstr(p, "\"");
	if (p == NULL) {
		if (fallback) {
			return NULL;
		}
		error("{red}No auth server found!\n");
	}
	p++;
	const char *q = strstr(p, "\"");
	if (q == NULL) {
		if (fallback) {
			return NULL;
		}
		error("{red}No auth server found!\n");
	}
	char *ret = malloc((size_t)(q - p + 1));
	strncpy(ret, p, (size_t)(q - p));
	ret[q - p] = '\0';
	return ret;
}
static char *get_service_from_header(const char *_Nonnull header, bool fallback)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get service from header.
	 * Example:
	 * www-authenticate: Bearer realm="https://auth.docker.io/token",service="registry.docker.io"
	 */
	const char *p = strstr_ignore_case(header, "wWw-aUthEntIcAtE: ");
	if (p == NULL) {
		if (fallback) {
			return NULL;
		}
		error("{red}No service found!\n");
	}
	p = strstr(p, "service=");
	if (p == NULL) {
		if (fallback) {
			return NULL;
		}
		error("{red}No service found!\n");
	}
	p = strstr(p, "\"");
	if (p == NULL) {
		if (fallback) {
			return NULL;
		}
		error("{red}No service found!\n");
	}
	p++;
	const char *q = strstr(p, "\"");
	if (q == NULL) {
		if (fallback) {
			return NULL;
		}
		error("{red}No service found!\n");
	}
	char *ret = malloc((size_t)(q - p + 1));
	strncpy(ret, p, (size_t)(q - p));
	ret[q - p] = '\0';
	return ret;
}
static char *get_auth_server_url(const char *_Nullable mirror, bool fallback)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get auth server url.
	 * Example:
	 * https://auth.docker.io/token?service=registry.docker.io
	 *
	 */
	if (mirror == NULL) {
		mirror = gloal_config.docker_mirror;
	}
	char url[4096] = { '\0' };
	sprintf(url, "https://%s/v2/", mirror);
	const char *curl_command[] = { "curl", "--max-time", "5", "-s", "-L", "-I", url, NULL };
	char *response = fork_execvp_get_stdout(curl_command);
	if (response == NULL) {
		if (fallback) {
			return NULL;
		}
		error("{red}Failed to get auth server!\n");
	}
	char *server = get_auth_server_from_header(response, fallback);
	if (server == NULL) {
		if (fallback) {
			free(response);
			return NULL;
		}
		error("{red}Failed to get auth server!\n");
	}
	char *service = get_service_from_header(response, fallback);
	if (service == NULL) {
		if (fallback) {
			free(response);
			free(server);
			return NULL;
		}
		error("{red}Failed to get service!\n");
	}
	free(response);
	char *ret = malloc(strlen(server) + strlen(service) + 22);
	sprintf(ret, "%s?service=%s", server, service);
	free(server);
	free(service);
	rurima_log("{base}Auth server url: {cyan}%s{clear}\n", ret);
	return ret;
}
static char *get_token(const char *_Nonnull image, const char *_Nullable mirror, bool fallback)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get token from Docker mirror.
	 * This token is used to pull other files of image.
	 *
	 */
	char url[4096] = { '\0' };
	if (mirror == NULL) {
		mirror = gloal_config.docker_mirror;
	}
	char *auth_server_url = get_auth_server_url(mirror, fallback);
	if (auth_server_url == NULL) {
		if (fallback) {
			rurima_log("{red}No auth server found, using homo magic token 1145141919810\n");
			// We hope the server administrator is homo.
			return strdup("1145141919810");
		} else {
			error("{red}Failed to get auth server!\n");
		}
	}
	strcat(url, auth_server_url);
	free(auth_server_url);
	strcat(url, "&scope=repository%3A");
	strcat(url, image);
	strcat(url, "%3Apull");
	rurima_log("{base}url: {cyan}%s{clear}\n", url);
	const char *curl_command[] = { "curl", "-L", "-s", url, NULL };
	char *token_json = fork_execvp_get_stdout(curl_command);
	if (token_json == NULL) {
		error("{red}Failed to get token!\n");
	}
	char *ret = json_get_key(token_json, "[token]");
	if (ret == NULL) {
		if (fallback) {
			free(token_json);
			rurima_log("{red}Can not get token, using homo magic token 1145141919810\n");
			// We hope the server administrator is homo.
			return strdup("1145141919810");
		} else {
			error("{red}Failed to get token!");
		}
	}
	free(token_json);
	rurima_log("{base}Token: {cyan}%s{clear}\n", ret);
	return ret;
}
static char *get_tag_manifests(const char *_Nonnull image, const char *_Nonnull tag, const char *_Nonnull token, const char *_Nullable mirror)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get manifests of image.
	 * This is used to get digest of image.
	 *
	 */
	if (mirror == NULL) {
		mirror = gloal_config.docker_mirror;
	}
	char url[4096] = { '\0' };
	sprintf(url, "https://%s/v2/%s/manifests/%s", mirror, image, tag);
	char *auth = malloc(strlen(token) + 114);
	auth[0] = '\0';
	sprintf(auth, "Authorization: Bearer %s", token);
	const char *curl_command[] = { "curl", "-L", "-s", "-H", "Accept: application/vnd.docker.distribution.manifest.list.v1+json", "-H", auth, url, NULL };
	char *ret = fork_execvp_get_stdout(curl_command);
	if (ret == NULL) {
		error("{red}Failed to get manifests!\n");
	}
	rurima_log("{base}Manifests: \n{cyan}%s{clear}\n", ret);
	free(auth);
	return ret;
}
static char *get_tag_digest(const char *_Nonnull manifests, const char *_Nullable architecture)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get digest of image.
	 * This is used to pull image.
	 *
	 */
	if (architecture == NULL) {
		architecture = docker_get_host_arch();
	}
	char *tmp = json_get_key(manifests, "[manifests]");
	char *digest = json_anon_layer_get_key(tmp, "[platform][architecture]", architecture, "[digest]");
	if (digest == NULL) {
		free(tmp);
		return NULL;
	}
	rurima_log("{base}Digest: %s{clear}\n", digest);
	free(tmp);
	return digest;
}
static char **get_blobs(const char *_Nonnull image, const char *_Nonnull digest, const char *_Nonnull token, const char *_Nullable mirror)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get blobs of image.
	 * This is used to pull image.
	 * The layers to get will be stored in the returned array.
	 *
	 */
	char url[4096] = { '\0' };
	if (mirror == NULL) {
		mirror = gloal_config.docker_mirror;
	}
	sprintf(url, "https://%s/v2/%s/manifests/%s", mirror, image, digest);
	rurima_log("{base}url: {cyan}%s{clear}\n", url);
	char *auth = malloc(strlen(token) + 114);
	auth[0] = '\0';
	sprintf(auth, "Authorization: Bearer %s", token);
	const char *curl_command[] = { "curl", "-L", "-s", "-H", "Accept: application/vnd.oci.image.manifest.v1+json", "-H", auth, url, NULL };
	char *response = fork_execvp_get_stdout(curl_command);
	rurima_log("{base}Response: \n{cyan}%s{clear}\n", response);
	if (response == NULL) {
		error("{red}Failed to get blobs!\n");
	}
	char *layers = json_get_key(response, "[layers]");
	if (layers == NULL) {
		error("{red}Failed to get layers!\n");
	}
	char **ret = NULL;
	size_t len = json_anon_layer_get_key_array(layers, "[digest]", &ret);
	if (len == 0) {
		error("{red}Failed to get layers!\n");
	}
	free(layers);
	free(response);
	free(auth);
	return ret;
}
static char *get_short_sha(const char *_Nonnull sha)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Just for displaying layer information.
	 *
	 */
	const char *p = &sha[7];
	char *ret = malloc(32);
	strncpy(ret, p, 16);
	ret[16] = '\0';
	return ret;
}
static void pull_images(const char *_Nonnull image, char *const *_Nonnull blobs, const char *_Nonnull token, const char *_Nonnull savedir, const char *_Nullable mirror, bool fallback)
{
	/*
	 * Pull images.
	 *
	 * This function will pull all layers of image,
	 * and extract them to savedir.
	 * Fallback mode will get token every time pull a layer.
	 *
	 */
	check_dir_deny_list(savedir);
	char *token_tmp = NULL;
	char url[4096] = { '\0' };
	char filename[4096] = { '\0' };
	if (mkdirs(savedir, 0755) != 0) {
		error("{red}Failed to create directory!\n");
	}
	chdir(savedir);
	if (mirror == NULL) {
		mirror = gloal_config.docker_mirror;
	}
	for (int i = 0;; i++) {
		if (blobs[i] == NULL) {
			break;
		}
		char *sha = get_short_sha(blobs[i]);
		cprintf("{base}Pulling{cyan} %s {base}as{cyan} layer-%d\n", sha, i);
		free(sha);
		sprintf(url, "https://%s/v2/%s/blobs/%s", mirror, image, blobs[i]);
		sprintf(filename, "layer-%d", i);
		if (fallback) {
			token_tmp = get_token(image, mirror, fallback);
		} else {
			token_tmp = strdup(token);
		}
		char *auth = malloc(strlen(token_tmp) + 114);
		auth[0] = '\0';
		sprintf(auth, "Authorization: Bearer %s", token_tmp);
		free(token_tmp);
		rurima_log("{base}Command:\n{cyan}curl -L -s -H \"%s\" %s -o %s\n", auth, url, filename);
		const char *curl_command[] = { "curl", "-L", "-s", "-H", auth, url, "-o", filename, NULL };
		int ret = fork_execvp(curl_command);
		if (ret != 0) {
			error("{red}Failed to pull image!\n");
		}
		free(auth);
		ret = extract_archive(filename, ".");
		if (ret != 0) {
			error("{red}Failed to extract archive!\n");
		}
		remove(filename);
	}
}
static char *get_config_digest_fallback(const char *_Nonnull image, const char *_Nonnull tag, const char *_Nonnull token, const char *_Nullable mirror)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Fallback to get config.
	 *
	 */
	if (mirror == NULL) {
		mirror = gloal_config.docker_mirror;
	}
	char *manifests = get_tag_manifests(image, tag, token, mirror);
	char *ret = json_get_key(manifests, "[config][digest]");
	if (ret == NULL) {
		error("{red}Failed to get config!\n");
	}
	free(manifests);
	return ret;
}
static char *get_config_digest(const char *_Nonnull image, const char *_Nonnull tag, const char *_Nullable digest, const char *_Nonnull token, const char *_Nullable mirror, bool fallback)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get the digest of config of image.
	 *
	 */
	if (mirror == NULL) {
		mirror = gloal_config.docker_mirror;
	}
	if (digest == NULL) {
		if (!fallback) {
			error("{red}No digest found!\n");
		}
		return get_config_digest_fallback(image, tag, token, mirror);
	}
	char url[4096] = { '\0' };
	sprintf(url, "https://%s/v2/%s/manifests/%s", mirror, image, digest);
	char *auth = malloc(strlen(token) + 114);
	sprintf(auth, "Authorization: Bearer %s", token);
	const char *curl_command[] = { "curl", "-L", "-s", "-H", "Accept: application/vnd.oci.image.manifest.v1+json", "-H", auth, url, NULL };
	char *response = fork_execvp_get_stdout(curl_command);
	if (response == NULL) {
		error("{red}Failed to get config!\n");
	}
	char *config = json_get_key(response, "[config][digest]");
	if (config == NULL) {
		free(response);
		free(auth);
		if (!fallback) {
			error("{red}Failed to get config!\n");
		}
		return get_config_digest_fallback(image, tag, token, mirror);
	}
	rurima_log("{base}Config: %s{clear}\n", config);
	free(response);
	free(auth);
	return config;
}
static char *env_get_right(const char *_Nonnull env)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * This is used to parse env to key and value.
	 *
	 */
	const char *p = env;
	for (size_t i = 0; i < strlen(env); i++) {
		if (env[i] == '\\') {
			i++;
			continue;
		}
		if (env[i] == '=') {
			p = &env[i + 1];
			break;
		}
	}
	return strdup(p);
}
static char *env_get_left(const char *_Nonnull env)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Same like above.
	 *
	 */
	char *ret = malloc(strlen(env) + 1);
	strcpy(ret, env);
	for (size_t i = 0; i < strlen(env); i++) {
		if (env[i] == '\\') {
			i++;
			continue;
		}
		if (env[i] == '=') {
			ret[i] = '\0';
			break;
		}
	}
	char *tmp = ret;
	ret = strdup(tmp);
	free(tmp);
	return ret;
}
static void parse_env(char *const *_Nonnull env, char **_Nonnull buf, int len)
{
	/*
	 * Parse env.
	 *
	 * Env is like KEY=VALUE,
	 * so we need to split them.
	 * This is because ruri use `-e ENV VALUE` to define env.
	 *
	 */
	if (len == 0) {
		return;
	}
	int j = 0;
	for (int i = 0; i < len; i++) {
		buf[j] = env_get_left(env[i]);
		buf[j + 1] = env_get_right(env[i]);
		buf[j + 2] = NULL;
		buf[j + 3] = NULL;
		j += 2;
	}
}
static struct DOCKER *get_image_config(const char *_Nonnull image, const char *_Nonnull config, const char *_Nonnull token, const char *_Nullable mirror)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get the config of image.
	 * return a struct DOCKER.
	 *
	 */
	struct DOCKER *ret = malloc(sizeof(struct DOCKER));
	if (mirror == NULL) {
		mirror = gloal_config.docker_mirror;
	}
	char url[4096] = { '\0' };
	sprintf(url, "https://%s/v2/%s/blobs/%s", mirror, image, config);
	char *auth = malloc(strlen(token) + 114);
	auth[0] = '\0';
	sprintf(auth, "Authorization: Bearer %s", token);
	const char *curl_command[] = { "curl", "-L", "-s", "-H", "Accept: application/vnd.oci.image.manifest.v1+json", "-H", auth, url, NULL };
	char *response = fork_execvp_get_stdout(curl_command);
	if (response == NULL) {
		error("{red}Failed to get config!\n");
	}
	rurima_log("{base}Config:\n{cyan} %s\n", response);
	{
		char *architecture = json_get_key(response, "[architecture]");
		rurima_log("{base}Env: {cyan}%s{clear}\n", architecture == NULL ? "NULL" : architecture);
		if (architecture == NULL) {
			ret->architecture = NULL;
		} else {
			ret->architecture = architecture;
		}
	}
	{
		char *workdir = json_get_key(response, "[config][WorkingDir]");
		rurima_log("{base}Env: {cyan}%s{clear}\n", workdir == NULL ? "NULL" : workdir);
		if (workdir == NULL) {
			ret->workdir = NULL;
		} else {
			ret->workdir = workdir;
		}
	}
	{
		char *env_from_json = json_get_key(response, "[config][Env]");
		rurima_log("{base}Env: {cyan}%s{clear}\n", env_from_json == NULL ? "NULL" : env_from_json);
		if (env_from_json != NULL) {
			char *tmp = malloc(strlen(env_from_json) + 114);
			sprintf(tmp, "env=%s\n", env_from_json);
			char *env[RURI_MAX_ENVS / 2];
			env[0] = NULL;
			int len = k2v_get_key(char_array, "env", tmp, env, RURI_MAX_ENVS / 2);
			parse_env(env, ret->env, len);
			for (int i = 0; i < len; i++) {
				rurima_log("{base}Env[%d]: {cyan}%s{clear}\n", i, env[i]);
				free(env[i]);
			}
			free(tmp);
			free(env_from_json);
		} else {
			ret->env[0] = NULL;
		}
	}
	{
		char *entrypoint = json_get_key(response, "[config][Entrypoint]");
		rurima_log("{base}Entrypoint: {cyan}%s{clear}\n", entrypoint == NULL ? "NULL" : entrypoint);
		if (entrypoint != NULL) {
			char *tmp = malloc(strlen(entrypoint) + 114);
			sprintf(tmp, "entrypoint=%s\n", entrypoint);
			int len = k2v_get_key(char_array, "entrypoint", tmp, ret->entrypoint, RURI_MAX_COMMANDS);
			for (int i = 0; i < len; i++) {
				rurima_log("{base}Entrypoint[%d]: {cyan}%s{clear}\n", i, ret->entrypoint[i]);
			}
			free(tmp);
			free(entrypoint);
		} else {
			ret->entrypoint[0] = NULL;
		}
	}
	{
		char *cmdline = json_get_key(response, "[config][Cmd]");
		rurima_log("{base}Cmdline: {cyan}%s{clear}\n", cmdline == NULL ? "NULL" : cmdline);
		if (cmdline != NULL) {
			char *tmp = malloc(strlen(cmdline) + 114);
			sprintf(tmp, "cmdline=%s\n", cmdline);
			int len = k2v_get_key(char_array, "cmdline", tmp, ret->command, RURI_MAX_COMMANDS);
			for (int i = 0; i < len; i++) {
				rurima_log("{base}Cmdline[%d]: {cyan}%s{clear}\n", i, ret->command[i]);
			}
			free(tmp);
			free(cmdline);
		} else {
			ret->command[0] = NULL;
		}
	}
	free(response);
	free(auth);
	return ret;
}
struct DOCKER *get_docker_config(const char *_Nonnull image, const char *_Nonnull tag, const char *_Nullable architecture, const char *_Nullable mirror, bool fallback)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Return the config of image.
	 * This function is called by subcommand.c
	 *
	 */
	char *token = get_token(image, mirror, fallback);
	char *manifests = get_tag_manifests(image, tag, token, mirror);
	char *digest = get_tag_digest(manifests, architecture);
	// digest can be NULL, then we go to fallback.
	char *config = get_config_digest(image, tag, digest, token, mirror, fallback);
	if (config == NULL) {
		error("{red}Failed to get config!\n");
	}
	struct DOCKER *ret = get_image_config(image, config, token, mirror);
	free(manifests);
	free(token);
	free(digest);
	free(config);
	return ret;
}
static struct DOCKER *docker_pull_fallback(const char *_Nonnull image, const char *_Nonnull tag, const char *_Nonnull savedir, const char *_Nullable mirror)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Fallback to pull image.
	 *
	 */
	if (mirror == NULL) {
		mirror = gloal_config.docker_mirror;
	}
	char *token = get_token(image, mirror, true);
	char *manifests = get_tag_manifests(image, tag, token, mirror);
	char **blobs = NULL;
	char *layers = json_get_key(manifests, "[layers]");
	size_t len = json_anon_layer_get_key_array(layers, "[digest]", &blobs);
	if (len == 0) {
		error("{red}Failed to get digest!\n");
	}
	pull_images(image, blobs, token, savedir, mirror, true);
	free(token);
	token = get_token(image, mirror, true);
	char *config = get_config_digest_fallback(image, tag, token, mirror);
	struct DOCKER *ret = get_image_config(image, config, token, mirror);
	free(manifests);
	free(token);
	for (size_t i = 0; blobs[i] != NULL; i++) {
		free(blobs[i]);
	}
	free(blobs);
	free(config);
	free(layers);
	return ret;
}
struct DOCKER *docker_pull(const char *_Nonnull image, const char *_Nonnull tag, const char *_Nullable architecture, const char *_Nonnull savedir, const char *_Nullable mirror, bool fallback)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * We pull all the layers of image,
	 * and extract them to savedir.
	 * And we return the config of image.
	 *
	 */
	if (mirror == NULL) {
		mirror = gloal_config.docker_mirror;
	}
	char *token = get_token(image, mirror, fallback);
	char *manifests = get_tag_manifests(image, tag, token, mirror);
	char *digest = get_tag_digest(manifests, architecture);
	if (digest == NULL) {
		free(manifests);
		free(token);
		if (!fallback) {
			error("{red}Failed to get digest!\n");
		}
		return docker_pull_fallback(image, tag, savedir, mirror);
	}
	char **blobs = get_blobs(image, digest, token, mirror);
	if (blobs == NULL) {
		error("{red}Failed to get blobs!\n");
	}
	pull_images(image, blobs, token, savedir, mirror, fallback);
	if (fallback) {
		free(token);
		token = get_token(image, mirror, true);
	}
	char *config = get_config_digest(image, tag, digest, token, mirror, fallback);
	if (fallback) {
		free(token);
		token = get_token(image, mirror, true);
	}
	struct DOCKER *ret = get_image_config(image, config, token, mirror);
	free(manifests);
	free(token);
	free(digest);
	for (size_t i = 0; blobs[i] != NULL; i++) {
		free(blobs[i]);
	}
	free(blobs);
	free(config);
	return ret;
}
static char *__docker_search(const char *_Nonnull url)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * This is the core parse function of docker_search().
	 * It will read info from url, and return next url.
	 *
	 */
	const char *curl_command[] = { "curl", "-L", "-s", url, NULL };
	char *response = fork_execvp_get_stdout(curl_command);
	if (response == NULL) {
		error("{red}Failed to get response from dockerhub!\n");
	}
	rurima_log("{base}Response from dockerhub:\n{cyan}%s{clear}\n", response);
	char *next_url = json_get_key(response, "[next]");
	char *results = json_get_key(response, "[results]");
	if (results == NULL) {
		error("{red}No results found!\n");
	}
	rurima_log("{base}Results:\n{cyan}%s{clear}\n", results);
	char **name = NULL;
	size_t len = json_anon_layer_get_key_array_allow_null_val(results, "[repo_name]", &name);
	if (len == 0) {
		error("{red}No results found!\n");
	}
	char **description = NULL;
	size_t len2 = json_anon_layer_get_key_array_allow_null_val(results, "[short_description]", &description);
	if (len2 != len) {
		error("{red}Incorrect json!\n");
	}
	char **is_offical = NULL;
	size_t len3 = json_anon_layer_get_key_array_allow_null_val(results, "[is_official]", &is_offical);
	if (len3 != len) {
		error("{red}Incorrect json!\n");
	}
	for (size_t i = 0; i < len; i++) {
		if (strcmp(is_offical[i], "true") == 0) {
			if (!gloal_config.quiet) {
				cprintf("{yellow}%s {green}[official]\n", name[i]);
				if (description[i] != NULL) {
					cprintf("  {cyan}Description: %s\n", description[i]);
				} else {
					cprintf("  {cyan}Description: No description\n");
				}
			} else {
				printf("%s [official]\n", name[i]);
			}
		} else {
			if (!gloal_config.quiet) {
				cprintf("{yellow}%s\n", name[i]);
				if (description[i] != NULL) {
					cprintf("  {cyan}Description: %s\n", description[i]);
				} else {
					cprintf("  {cyan}Description: No description\n");
				}
			} else {
				printf("%s\n", name[i]);
			}
		}
	}
	free(response);
	for (size_t i = 0; i < len; i++) {
		free(name[i]);
		free(description[i]);
		free(is_offical[i]);
	}
	free(results);
	free(name);
	free(description);
	free(is_offical);
	return next_url;
}
int docker_search(const char *_Nonnull image, const char *_Nonnull page_size, bool quiet)
{
	/*
	 *
	 * An implementation of docker search.
	 * Return value is not important here,
	 * because we will error() directly if failed.
	 *
	 */
	char *url = malloc(4096);
	url[0] = '\0';
	strcat(url, "https://hub.docker.com/v2/search/repositories/?page_size=");
	strcat(url, page_size);
	strcat(url, "&query=");
	strcat(url, image);
	while (true) {
		rurima_log("{base}url: {cyan}%s{clear}\n", url);
		char *next_url = __docker_search(url);
		if (next_url == NULL) {
			exit(EXIT_SUCCESS);
		}
		rurima_log("{base}nexturl: {cyan}%s{clear}\n", next_url);
		if (quiet) {
			free(url);
			free(next_url);
			break;
		}
		char goto_next[114] = { '\0' };
		get_input("\n{purple}Continue see more results? (y/n): ", goto_next);
		rurima_log("{base}goto_next: {cyan}%s{clear}\n", goto_next);
		if (strcmp(goto_next, "y") == 0) {
			free(url);
			url = next_url;
			rurima_log("{base}next_url: {cyan}%s{clear}\n", next_url);
		} else {
			free(url);
			free(next_url);
			break;
		}
	}
	return 0;
}
static char *__docker_search_tag(const char *_Nonnull image, const char *_Nonnull url, const char *_Nullable architecture)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * This is the core parse function of docker_search_tag().
	 * It will read info from url, and return next url.
	 *
	 */
	rurima_log("{base}url: {cyan}%s{clear}\n", url);
	const char *curl_command[] = { "curl", "-L", "-s", url, NULL };
	char *response = fork_execvp_get_stdout(curl_command);
	if (response == NULL) {
		error("{red}Failed to get response from dockerhub!\n");
	}
	rurima_log("{base}Response from dockerhub:\n{cyan}%s{clear}\n", response);
	if (architecture == NULL) {
		architecture = docker_get_host_arch();
	}
	char *next_url = json_get_key(response, "[next]");
	rurima_log("{base}next_url: {cyan}%s{clear}\n", next_url);
	char *results = json_get_key(response, "[results]");
	char **images = NULL;
	size_t len = json_anon_layer_get_key_array_allow_null_val(results, "[images]", &images);
	if (len == 0) {
		error("{red}No results found!\n");
	}
	char **tags = NULL;
	size_t len2 = json_anon_layer_get_key_array_allow_null_val(results, "[name]", &tags);
	if (len2 != len) {
		printf("len: %zu, len2: %zu\n", len, len2);
		error("{red}Incorrect json!\n");
	}
	bool found = false;
	char *tmp = NULL;
	for (size_t i = 0; i < len; i++) {
		if (images[i] == NULL || tags[i] == NULL) {
			continue;
		}
		tmp = json_anon_layer_get_key(images[i], "[architecture]", architecture, "[digest]");
		if (tmp != NULL) {
			found = true;
			if (!gloal_config.quiet) {
				cprintf("{yellow}[%s]: {cyan}%s{clear}\n", image, tags[i]);
			} else {
				printf("[%s]: %s\n", image, tags[i]);
			}
			free(tmp);
		}
	}
	if (!found) {
		error("{red}No results found!\n");
	}
	free(response);
	free(results);
	for (size_t i = 0; i < len; i++) {
		free(images[i]);
		free(tags[i]);
	}
	free(images);
	free(tags);
	return next_url;
}
int docker_search_tag(const char *_Nonnull image, const char *_Nonnull page_size, const char *_Nullable architecture, bool quiet)
{
	/*
	 * An implementation of docker tag search.
	 *
	 * The return value is not important here,
	 * because we will error() directly if failed.
	 *
	 */
	char *url = malloc(4096);
	url[0] = '\0';
	strcat(url, "https://hub.docker.com/v2/repositories/");
	strcat(url, image);
	strcat(url, "/tags/?page_size=");
	strcat(url, page_size);
	while (true) {
		rurima_log("{base}url: {cyan}%s{clear}\n", url);
		char *next_url = __docker_search_tag(image, url, architecture);
		if (next_url == NULL) {
			exit(EXIT_SUCCESS);
		}
		rurima_log("{base}nexturl: {cyan}%s{clear}\n", next_url);
		if (quiet) {
			free(url);
			free(next_url);
			break;
		}
		char goto_next[114] = { '\0' };
		get_input("\n{purple}Continue see more results? (y/n): ", goto_next);
		rurima_log("{base}goto_next: {cyan}%s{clear}\n", goto_next);
		if (strcmp(goto_next, "y") == 0) {
			free(url);
			url = next_url;
			rurima_log("{base}next_url: {cyan}%s{clear}\n", next_url);
		} else {
			free(url);
			free(next_url);
			break;
		}
	}
	return 0;
}
static void __docker_add_archlist(char *_Nonnull arch, char ***_Nullable archlist)
{
	/*
	 * Add arch to archlist.
	 *
	 * We will not add duplicate arch.
	 *
	 */
	if (strcmp(arch, "unknown") == 0) {
		return;
	}
	if (*archlist == NULL) {
		*archlist = malloc(sizeof(char *) * 3);
		(*archlist)[0] = strdup(arch);
		(*archlist)[1] = NULL;
		return;
	}
	for (int i = 0; (*archlist)[i] != NULL; i++) {
		if (strcmp((*archlist)[i], arch) == 0) {
			return;
		}
	}
	size_t j = 0;
	for (; (*archlist)[j] != NULL; j++)
		;
	*archlist = realloc(*archlist, sizeof(char *) * (j + 3));
	for (int i = 0;; i++) {
		if ((*archlist)[i] == NULL) {
			(*archlist)[i] = strdup(arch);
			(*archlist)[i + 1] = NULL;
			break;
		}
	}
}
static void docker_print_arch(const char *_Nonnull image, char *const *_Nonnull arch, size_t len)
{
	/*
	 * Print architecture of image.
	 *
	 * We will not print unknown architecture or duplicate architecture.
	 *
	 */
	char **archlist = NULL;
	for (size_t i = 0; i < len; i++) {
		__docker_add_archlist(arch[i], &archlist);
	}
	if (archlist == NULL) {
		error("{red}No results found!\n");
	}
	for (int i = 0; archlist[i] != NULL; i++) {
		if (!gloal_config.quiet) {
			cprintf("{yellow}[%s]: {cyan}%s{clear}\n", image, archlist[i]);
		} else {
			printf("[%s]: %s\n", image, archlist[i]);
		}
	}
	for (int i = 0; archlist[i] != NULL; i++) {
		free(archlist[i]);
	}
	free(archlist);
}
int docker_search_arch(const char *_Nonnull image, const char *_Nonnull tag, char *_Nullable mirror, bool fallback)
{
	/*
	 * Get architecture of image:tag.
	 *
	 * The return value is not important here,
	 * because we will error() directly if failed.
	 *
	 */
	if (mirror == NULL) {
		mirror = gloal_config.docker_mirror;
	}
	char *token = get_token(image, mirror, fallback);
	char *manifests = get_tag_manifests(image, tag, token, mirror);
	char **arch = NULL;
	char *tmp = json_get_key(manifests, "[manifests]");
	size_t len = json_anon_layer_get_key_array(tmp, "[platform][architecture]", &arch);
	if (len == 0) {
		error("{red}No results found!\n");
	}
	docker_print_arch(image, arch, len);
	free(manifests);
	free(token);
	for (size_t i = 0; i < len; i++) {
		free(arch[i]);
	}
	free(arch);
	free(tmp);
	return 0;
}