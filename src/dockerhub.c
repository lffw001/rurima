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
static char *get_token(const char *_Nonnull image)
{
	/*
	 * Warning: free() the return value after use.
	 */
	char url[4096] = { '\0' };
	strcat(url, "https://auth.docker.io/token?service=registry.docker.io&scope=repository%3A");
	strcat(url, image);
	strcat(url, "%3Apull");
	const char *curl_command[] = { "curl", "-L", "-s", url, NULL };
	char *token_json = fork_execvp_get_stdout(curl_command);
	if (token_json == NULL) {
		error("{red}Failed to get token!\n");
	}
	char *ret = json_get_key(token_json, "[token]");
	free(token_json);
	log("{base}Token: {cyan}%s{clear}\n", ret);
	return ret;
}
static char *get_tag_manifests(const char *_Nonnull image, const char *_Nonnull tag, const char *_Nonnull token)
{
	/*
	 * Warning: free() the return value after use.
	 */
	char url[4096] = { '\0' };
	strcat(url, "https://registry-1.docker.io/v2/");
	strcat(url, image);
	strcat(url, "/manifests/");
	strcat(url, tag);
	char *auth = malloc(strlen(token) + 114);
	auth[0] = '\0';
	sprintf(auth, "Authorization: Bearer %s", token);
	const char *curl_command[] = { "curl", "-L", "-s", "-H", "Accept: application/vnd.docker.distribution.manifest.list.v2+json", "-H", auth, url, NULL };
	char *ret = fork_execvp_get_stdout(curl_command);
	if (ret == NULL) {
		error("{red}Failed to get manifests!\n");
	}
	log("{base}Manifests: \n{cyan}%s{clear}\n", ret);
	free(auth);
	return ret;
}
static char *get_tag_digest(const char *_Nonnull manifests, const char *_Nullable architecture)
{
	/*
	 * Warning: free() the return value after use.
	 */
	if (architecture == NULL) {
		architecture = get_host_arch();
	}
	char *tmp = json_get_key(manifests, "[manifests]");
	char *digest = json_anon_layer_get_key(tmp, "[platform][architecture]", architecture, "[digest]");
	if (digest == NULL) {
		error("{red}No digest found!\n");
	}
	log("{base}Digest: %s{clear}\n", digest);
	free(tmp);
	return digest;
}
static char **get_blobs(const char *_Nonnull image, const char *_Nonnull digest, const char *_Nonnull token)
{
	/*
	 * Warning: free() the return value after use.
	 */
	char url[4096] = { '\0' };
	strcat(url, "https://registry-1.docker.io/v2/");
	strcat(url, image);
	strcat(url, "/manifests/");
	strcat(url, digest);
	log("{base}url: {cyan}%s{clear}\n", url);
	char *auth = malloc(strlen(token) + 114);
	auth[0] = '\0';
	sprintf(auth, "Authorization: Bearer %s", token);
	const char *curl_command[] = { "curl", "-L", "-s", "-H", "Accept: application/vnd.oci.image.manifest.v1+json", "-H", auth, url, NULL };
	char *response = fork_execvp_get_stdout(curl_command);
	log("{base}Response: \n{cyan}%s{clear}\n", response);
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
	 */
	const char *p = &sha[7];
	char *ret = malloc(32);
	strncpy(ret, p, 16);
	ret[16] = '\0';
	return ret;
}
static void pull_images(const char *_Nonnull image, char *const *_Nonnull blobs, const char *_Nonnull token, const char *_Nonnull savedir)
{
	/*
	 * Pull images.
	 */
	char url[4096] = { '\0' };
	char filename[4096] = { '\0' };
	if (mkdirs(savedir, 0755) != 0) {
		error("{red}Failed to create directory!\n");
	}
	chdir(savedir);
	for (int i = 0;; i++) {
		if (blobs[i] == NULL) {
			break;
		}
		char *sha = get_short_sha(blobs[i]);
		cprintf("{base}Pulling{cyan} %s {base}as{cyan} layer-%d\n", sha, i);
		free(sha);
		sprintf(url, "https://registry-1.docker.io/v2/%s/blobs/%s", image, blobs[i]);
		sprintf(filename, "layer-%d", i);
		char *auth = malloc(strlen(token) + 114);
		auth[0] = '\0';
		sprintf(auth, "Authorization: Bearer %s", token);
		log("{base}Command:\n{cyan}curl -L -s -H \"%s\" %s -o %s\n", auth, url, filename);
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
static char *get_config_digset(const char *_Nonnull image, const char *_Nonnull digest, const char *_Nonnull token)
{
	/*
	 * Warning: free() the return value after use.
	 */
	char url[4096] = { '\0' };
	strcat(url, "https://registry-1.docker.io/v2/");
	strcat(url, image);
	strcat(url, "/manifests/");
	strcat(url, digest);
	char *auth = malloc(strlen(token) + 114);
	sprintf(auth, "Authorization: Bearer %s", token);
	const char *curl_command[] = { "curl", "-L", "-s", "-H", "Accept: application/vnd.oci.image.manifest.v1+json", "-H", auth, url, NULL };
	char *response = fork_execvp_get_stdout(curl_command);
	if (response == NULL) {
		error("{red}Failed to get config!\n");
	}
	char *config = json_get_key(response, "[config][digest]");
	if (config == NULL) {
		error("{red}Failed to get config!\n");
	}
	log("{base}Config: %s{clear}\n", config);
	free(response);
	free(auth);
	return config;
}
static char *env_get_right(const char *_Nonnull env)
{
	/*
	 * Warning: free() the return value after use.
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
static struct DOCKER *get_image_config(const char *_Nonnull image, const char *_Nonnull config, const char *_Nonnull token)
{
	/*
	 * Warning: free() the return value after use.
	 */
	struct DOCKER *ret = malloc(sizeof(struct DOCKER));
	char url[4096] = { '\0' };
	strcat(url, "https://registry-1.docker.io/v2/");
	strcat(url, image);
	strcat(url, "/blobs/");
	strcat(url, config);
	char *auth = malloc(strlen(token) + 114);
	auth[0] = '\0';
	sprintf(auth, "Authorization: Bearer %s", token);
	const char *curl_command[] = { "curl", "-L", "-s", "-H", "Accept: application/vnd.oci.image.manifest.v1+json", "-H", auth, url, NULL };
	char *response = fork_execvp_get_stdout(curl_command);
	if (response == NULL) {
		error("{red}Failed to get blobs!\n");
	}
	{
		char *workdir = json_get_key(response, "[config][WorkingDir]");
		log("{base}Env: {cyan}%s{clear}\n", workdir == NULL ? "NULL" : workdir);
		if (workdir == NULL) {
			ret->workdir = NULL;
		} else {
			ret->workdir = workdir;
		}
	}
	{
		char *env_from_json = json_get_key(response, "[config][Env]");
		log("{base}Env: {cyan}%s{clear}\n", env_from_json == NULL ? "NULL" : env_from_json);
		if (env_from_json != NULL) {
			char *tmp = malloc(strlen(env_from_json) + 114);
			sprintf(tmp, "env=%s\n", env_from_json);
			char *env[MAX_ENVS];
			env[0] = NULL;
			int len = k2v_get_key(char_array, "env", tmp, env, MAX_ENVS);
			parse_env(env, ret->env, len);
			for (int i = 0; i < len; i++) {
				log("{base}Env[%d]: {cyan}%s{clear}\n", i, env[i]);
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
		log("{base}Entrypoint: {cyan}%s{clear}\n", entrypoint == NULL ? "NULL" : entrypoint);
		if (entrypoint != NULL) {
			char *tmp = malloc(strlen(entrypoint) + 114);
			sprintf(tmp, "entrypoint=%s\n", entrypoint);
			int len = k2v_get_key(char_array, "entrypoint", tmp, ret->entrypoint, MAX_COMMANDS);
			for (int i = 0; i < len; i++) {
				log("{base}Entrypoint[%d]: {cyan}%s{clear}\n", i, ret->entrypoint[i]);
			}
			free(tmp);
			free(entrypoint);
		} else {
			ret->entrypoint[0] = NULL;
		}
	}
	{
		char *cmdline = json_get_key(response, "[config][Cmd]");
		log("{base}Cmdline: {cyan}%s{clear}\n", cmdline == NULL ? "NULL" : cmdline);
		if (cmdline != NULL) {
			char *tmp = malloc(strlen(cmdline) + 114);
			sprintf(tmp, "cmdline=%s\n", cmdline);
			int len = k2v_get_key(char_array, "cmdline", tmp, ret->command, MAX_COMMANDS);
			for (int i = 0; i < len; i++) {
				log("{base}Cmdline[%d]: {cyan}%s{clear}\n", i, ret->command[i]);
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
struct DOCKER *get_config(const char *_Nonnull image, const char *_Nonnull tag, const char *_Nullable architecture)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Return the config of image.
	 */
	char *token = get_token(image);
	char *manifests = get_tag_manifests(image, tag, token);
	char *digest = get_tag_digest(manifests, architecture);
	char *config = get_config_digset(image, digest, token);
	struct DOCKER *ret = get_image_config(image, config, token);
	free(manifests);
	free(token);
	free(digest);
	free(config);
	return ret;
}
struct DOCKER *docker_pull(const char *_Nonnull image, const char *_Nonnull tag, const char *_Nullable architecture, const char *_Nonnull savedir)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Return the config of image.
	 */
	char *token = get_token(image);
	char *manifests = get_tag_manifests(image, tag, token);
	char *digest = get_tag_digest(manifests, architecture);
	char **blobs = get_blobs(image, digest, token);
	if (blobs == NULL) {
		error("{red}Failed to get blobs!\n");
	}
	pull_images(image, blobs, token, savedir);
	char *config = get_config_digset(image, digest, token);
	struct DOCKER *ret = get_image_config(image, config, token);
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
	 * Return next url.
	 */
	const char *curl_command[] = { "curl", "-L", "-s", url, NULL };
	char *response = fork_execvp_get_stdout(curl_command);
	if (response == NULL) {
		error("{red}Failed to get response from dockerhub!\n");
	}
	log("{base}Response from dockerhub:\n{cyan}%s{clear}\n", response);
	char *next_url = json_get_key(response, "[next]");
	char *results = json_get_key(response, "[results]");
	if (results == NULL) {
		error("{red}No results found!\n");
	}
	log("{base}Results:\n{cyan}%s{clear}\n", results);
	char **name = NULL;
	size_t len = json_anon_layer_get_key_array(results, "[repo_name]", &name);
	if (len == 0) {
		error("{red}No results found!\n");
	}
	char **description = NULL;
	size_t len2 = json_anon_layer_get_key_array(results, "[short_description]", &description);
	if (len2 != len) {
		error("{red}Incorrect json!\n");
	}
	char **is_offical = NULL;
	size_t len3 = json_anon_layer_get_key_array(results, "[is_official]", &is_offical);
	if (len3 != len) {
		error("{red}Incorrect json!\n");
	}
	for (size_t i = 0; i < len; i++) {
		if (strcmp(is_offical[i], "true") == 0) {
			cprintf("{yellow}%s {green}[official]\n", name[i]);
			if (description[i] != NULL) {
				cprintf("  {cyan}Description: %s\n", description[i]);
			} else {
				cprintf("  {cyan}Description: No description\n");
			}
		} else {
			cprintf("{yellow}%s\n", name[i]);
			if (description[i] != NULL) {
				cprintf("  {cyan}Description: %s\n", description[i]);
			} else {
				cprintf("  {cyan}Description: No description\n");
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
	char *url = malloc(4096);
	url[0] = '\0';
	strcat(url, "https://hub.docker.com/v2/search/repositories/?page_size=");
	strcat(url, page_size);
	strcat(url, "&query=");
	strcat(url, image);
	while (true) {
		log("{base}url: {cyan}%s{clear}\n", url);
		char *next_url = __docker_search(url);
		log("{base}nexturl: {cyan}%s{clear}\n", next_url);
		if (quiet) {
			free(url);
			free(next_url);
			break;
		}
		char goto_next[114] = { '\0' };
		get_input("\n{purple}Continue see more results? (y/n): ", goto_next);
		log("{base}goto_next: {cyan}%s{clear}\n", goto_next);
		if (strcmp(goto_next, "y") == 0) {
			free(url);
			url = next_url;
			log("{base}next_url: {cyan}%s{clear}\n", next_url);
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
	 * Return next url.
	 */
	log("{base}url: {cyan}%s{clear}\n", url);
	const char *curl_command[] = { "curl", "-L", "-s", url, NULL };
	char *response = fork_execvp_get_stdout(curl_command);
	if (response == NULL) {
		error("{red}Failed to get response from dockerhub!\n");
	}
	log("{base}Response from dockerhub:\n{cyan}%s{clear}\n", response);
	if (architecture == NULL) {
		architecture = get_host_arch();
	}
	char *next_url = json_get_key(response, "[next]");
	log("{base}next_url: {cyan}%s{clear}\n", next_url);
	char *results = json_get_key(response, "[results]");
	char **images = NULL;
	size_t len = json_anon_layer_get_key_array(results, "[images]", &images);
	if (len == 0) {
		error("{red}No results found!\n");
	}
	char **tags = NULL;
	size_t len2 = json_anon_layer_get_key_array(results, "[name]", &tags);
	if (len2 != len) {
		error("{red}Incorrect json!\n");
	}
	bool found = false;
	char *tmp = NULL;
	for (size_t i = 0; i < len; i++) {
		tmp = json_anon_layer_get_key(images[i], "[architecture]", architecture, "[digest]");
		if (tmp != NULL) {
			found = true;
			cprintf("{yellow}[%s]: {cyan}%s{clear}\n", image, tags[i]);
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
	char *url = malloc(4096);
	url[0] = '\0';
	strcat(url, "https://hub.docker.com/v2/repositories/");
	strcat(url, image);
	strcat(url, "/tags/?page_size=");
	strcat(url, page_size);
	while (true) {
		log("{base}url: {cyan}%s{clear}\n", url);
		char *next_url = __docker_search_tag(image, url, architecture);
		log("{base}nexturl: {cyan}%s{clear}\n", next_url);
		if (quiet) {
			free(url);
			free(next_url);
			break;
		}
		char goto_next[114] = { '\0' };
		get_input("\n{purple}Continue see more results? (y/n): ", goto_next);
		log("{base}goto_next: {cyan}%s{clear}\n", goto_next);
		if (strcmp(goto_next, "y") == 0) {
			free(url);
			url = next_url;
			log("{base}next_url: {cyan}%s{clear}\n", next_url);
		} else {
			free(url);
			free(next_url);
			break;
		}
	}
	return 0;
}