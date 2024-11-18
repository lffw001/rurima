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
#include "include/jsonv.h"
#include "include/rurima.h"
/*
 * If there's no bugs, do not care how it works,
 * just use it.
 */
static char *unicode_to_char(const char *_Nonnull str)
{
	/*
	 * Warning: free() after use.
	 *
	 * This is used to convert unicode to char.
	 * We find unicode format like \uXXXX and convert it to char.
	 *
	 */
	size_t len = strlen(str);
	char *result = malloc(len + 1);
	if (!result) {
		return NULL;
	}
	size_t j = 0;
	for (size_t i = 0; i < len; i++) {
		if (str[i] == '\\' && i < len - 5 && str[i + 1] == 'u' && isxdigit(str[i + 2]) && isxdigit(str[i + 3]) && isxdigit(str[i + 4]) && isxdigit(str[i + 5])) {
			char hex[5] = { str[i + 2], str[i + 3], str[i + 4], str[i + 5], '\0' };
			result[j++] = (char)strtol(hex, NULL, 16);
			log("{base}unicode: {cyan}%s{clear}\n", hex);
			i += 5;
		} else {
			result[j++] = str[i];
		}
	}
	result[j] = '\0';
	log("{base}Result:\n{cyan}%s\n", result);
	return result;
}
static char *format_json(const char *_Nonnull buf)
{
	/*
	 * Warning: free() after use.
	 *
	 * We convert unicode to char,
	 * and remove comments.
	 *
	 * We will not correct backslashes (`\\` to `\`),
	 * it is corrected before return when we get the key.
	 *
	 */
	char *tmp = unicode_to_char(buf);
	char *ret = malloc(strlen(tmp) + 1);
	size_t j = 0;
	bool in_string = false;
	for (size_t i = 0; i < strlen(tmp); i++) {
		if (tmp[i] == '\\') {
			ret[j] = tmp[i];
			ret[j + 1] = '\0';
			i++;
			j++;
			if (i >= strlen(tmp)) {
				break;
			}
			ret[j] = tmp[i];
			ret[j + 1] = '\0';
			j++;
			continue;
		} else if (tmp[i] == '"') {
			in_string = !in_string;
		} else if (!in_string && tmp[i] == '/' && i + 1 < strlen(tmp) && tmp[i + 1] == '/') {
			while (i < strlen(tmp) && tmp[i] != '\n') {
				i++;
			}
			i++;
		} else if (!in_string && tmp[i] == '/' && i + 1 < strlen(tmp) && tmp[i + 1] == '*') {
			i += 2;
			while (!in_string && tmp[i] != '*' && i + 1 < strlen(tmp) && tmp[i + 1] != '/') {
				i++;
			}
			i += 2;
			if (i + 1 < strlen(tmp)) {
				i++;
			}
		}
		ret[j] = tmp[i];
		ret[j + 1] = '\0';
		j++;
	}
	free(tmp);
	log("{base}ret:{cyan}\n%s\n", ret);
	return ret;
}
static char *next_key(const char *_Nullable buf)
{
	/*
	 * Need not to free() after use.
	 *
	 * Find the start of the next key,
	 * and return the pointer to the start of the key.
	 * If no key is found, return NULL.
	 */
	if (buf == NULL) {
		return NULL;
	}
	if (strlen(buf) == 0) {
		return NULL;
	}
	const char *p = buf;
	// Reach the first key.
	for (size_t i = 0; i < strlen(p); i++) {
		if (p[i] == '\\') {
			i++;
			continue;
		} else if (p[i] == '"') {
			p = &p[i];
			break;
		}
	}
	int level = 0;
	bool in_string = false;
	// Get the next key.
	for (size_t i = 0; i < strlen(p); i++) {
		if (p[i] == '\\') {
			i++;
			continue;
		} else if (p[i] == '"') {
			in_string = !in_string;
		} else if ((p[i] == '{' || p[i] == '[') && !in_string) {
			level++;
		} else if ((p[i] == '}' || p[i] == ']') && !in_string) {
			level--;
			if (level == -1) {
				return NULL;
			}
		} else if (p[i] == ',' && !in_string && level == 0) {
			if (i < strlen(p) - 1) {
				return (char *)&p[i + 1];
			} else {
				return NULL;
			}
		}
	}
	return NULL;
}
static char *next_layer(const char *_Nullable buf)
{
	/*
	 * Need not to free() after use.
	 *
	 * Find the start of the next anon layer,
	 * and return the pointer to the start of the layer.
	 * If no layer is found, return NULL.
	 *
	 */
	if (buf == NULL) {
		return NULL;
	}
	const char *p = buf;
	// Reach the first layer.
	for (size_t i = 0; i < strlen(p); i++) {
		if (p[i] == '\\') {
			i++;
			continue;
		} else if (p[i] == '{') {
			p = &p[i];
			break;
		}
	}
	int level = 0;
	bool in_string = false;
	// Get the next key.
	for (size_t i = 0; i < strlen(p); i++) {
		if (p[i] == '\\') {
			i++;
			continue;
		} else if (p[i] == '"') {
			in_string = !in_string;
		} else if ((p[i] == '{' || p[i] == '[') && !in_string) {
			level++;
		} else if ((p[i] == '}' || p[i] == ']') && !in_string) {
			level--;
			if (level == -1) {
				return NULL;
			}
		} else if (p[i] == ',' && !in_string && level == 0) {
			if (i < strlen(p) - 1) {
				return (char *)&p[i + 1];
			} else {
				return NULL;
			}
		}
	}
	return NULL;
}
static char *current_key(const char *_Nonnull buf)
{
	/*
	 * Warning: free() after use.
	 *
	 * Find the current key,
	 * and return the key.
	 * If no key is found, return NULL.
	 *
	 */
	char *tmp = strdup(buf);
	char *ret = NULL;
	// Skip space.
	for (size_t i = 0; i < strlen(tmp); i++) {
		if (tmp[i] == '\\') {
			i++;
			continue;
		} else if (tmp[i] == '"') {
			ret = &tmp[i + 1];
			break;
		}
	}
	if (ret == NULL) {
		free(tmp);
		return NULL;
	}
	for (size_t i = 0; i < strlen(ret); i++) {
		if (ret[i] == '\\') {
			i++;
			continue;
		} else if (ret[i] == '"' && i < strlen(ret)) {
			ret[i] = '\0';
			break;
		}
		if (i == strlen(ret) - 1) {
			free(tmp);
			return NULL;
		}
	}
	ret = strdup(ret);
	free(tmp);
	return ret;
}
static char *correct_backslash(const char *_Nullable buf)
{
	/*
	 * Warning: free() after use.
	 *
	 * Correct backslashes, `\\` to `\`.
	 * This function is used by json_get_key().
	 *
	 */
	if (buf == NULL) {
		return NULL;
	}
	char *ret = malloc(strlen(buf) + 1);
	size_t j = 0;
	for (size_t i = 0; i < strlen(buf); i++) {
		if (buf[i] == '\\' && i < strlen(buf) - 1 && buf[i + 1] == '\\') {
			i++;
		}
		ret[j] = buf[i];
		ret[j + 1] = '\0';
		j++;
	}
	char *tmp = ret;
	ret = strdup(tmp);
	free(tmp);
	log("{base}ret: \n{cyan}%s\n", ret);
	return ret;
}
static char *parse_value(const char *_Nullable buf)
{
	/*
	 * Warning: free() after use.
	 *
	 * Parse the value we get, we will do:
	 * NULL value check.
	 * Empty value check.
	 * Remove space in the front.
	 * Remove quotes if the value is a string.
	 *
	 */
	log("{base}buf:\n{cyan}%s\n", buf);
	if (buf == NULL) {
		return NULL;
	}
	char *tmp = strdup(buf);
	if (tmp == NULL) {
		return NULL;
	}
	if (strcmp(tmp, "\"\"") == 0) {
		free(tmp);
		return NULL;
	}
	if (strcmp(tmp, "null") == 0) {
		free(tmp);
		return NULL;
	}
	if (strcmp(tmp, "[]") == 0) {
		free(tmp);
		return NULL;
	}
	if (strcmp(tmp, "{}") == 0) {
		free(tmp);
		return NULL;
	}
	char *ret = NULL;
	// Skip space.
	for (size_t i = 0; i < strlen(tmp); i++) {
		if (tmp[i] == '\\') {
			i++;
			continue;
		} else if (tmp[i] == ' ') {
			continue;
		} else if (tmp[i] == '[' || tmp[i] == '{') {
			ret = strdup(&tmp[i]);
			free(tmp);
			return ret;
		} else if (tmp[i] == '"' && i < strlen(tmp)) {
			ret = &tmp[i + 1];
			break;
		} else {
			ret = strdup(&tmp[i]);
			free(tmp);
			return ret;
			break;
		}
	}
	if (ret == NULL) {
		free(tmp);
		return NULL;
	}
	for (size_t i = 0; i < strlen(ret); i++) {
		if (ret[i] == '\\') {
			i++;
			continue;
		} else if (ret[i] == '"') {
			ret[i] = '\0';
			break;
		}
		if (i == strlen(ret) - 1) {
			free(tmp);
			return NULL;
		}
	}
	ret = strdup(ret);
	free(tmp);
	return ret;
}
static char *current_value(const char *_Nonnull buf)
{
	/*
	 * Warning: free() after use.
	 *
	 * Find the current value,
	 * and return the value.
	 * If no value is found, return NULL.
	 *
	 */
	int level = 0;
	char *tmp = strdup(buf);
	char *ret = NULL;
	// Skip key.
	bool in_string = false;
	for (size_t i = 0; i < strlen(buf); i++) {
		if (buf[i] == '\\') {
			i++;
			continue;
		} else if (buf[i] == '"') {
			in_string = !in_string;
		} else if (buf[i] == ':' && !in_string) {
			ret = &tmp[i + 1];
			break;
		}
	}
	if (ret == NULL) {
		free(tmp);
		return NULL;
	}
	// Skip space.
	for (size_t i = 0; i < strlen(ret); i++) {
		if (ret[i] == '\\') {
			i++;
			continue;
		} else if (ret[i] != ' ' && ret[i] != '\n') {
			ret = &ret[i];
			break;
		}
	}
	for (size_t i = 0; i < strlen(ret); i++) {
		if (ret[i] == '\\') {
			i++;
			continue;
		} else if (ret[i] == '"') {
			in_string = !in_string;
		} else if ((ret[i] == '{' || ret[i] == '[') && !in_string) {
			level++;
		} else if ((ret[i] == '}' || ret[i] == ']') && !in_string) {
			level--;
			// The last value might not have a comma.
			if (level == -1) {
				ret[i] = '\0';
				break;
			}
		} else if (ret[i] == ',' && !in_string && level == 0) {
			if (i < strlen(ret) - 1) {
				ret[i] = '\0';
				break;
			} else {
				free(tmp);
				return NULL;
			}
		}
	}
	ret = parse_value(ret);
	free(tmp);
	if (ret != NULL) {
		log("{base}Current value: {cyan}%s{clear}\n", ret);
	} else {
		log("{base}Current value: {cyan}NULL{clear}\n");
	}
	return ret;
}
static char *json_get_key_one_level(const char *_Nonnull buf, const char *_Nonnull key)
{
	/*
	 * Warning: free() after use.
	 *
	 * From one json layer, get the value of key.
	 * Return: The value we get.
	 * If we didn't find the key, return NULL.
	 *
	 */
	const char *p = buf;
	const char *q = p;
	char *current = NULL;
	while (p != NULL) {
		current = current_key(p);
		if (current == NULL) {
			break;
		}
		if (strcmp(current, key) == 0) {
			char *ret = current_value(p);
			free(current);
			return ret;
		}
		free(current);
		q = p;
		p = next_key(p);
		if (p == NULL) {
			p = next_layer(q);
		}
	}
	return NULL;
}
char *json_get_key(const char *_Nonnull buf, const char *_Nonnull key)
{
	/*
	 * Example json:
	 * {"foo":
	 *   {"bar":
	 *     {"buz":"xxxx"
	 *     }
	 *   }
	 * }
	 * We use key [foo][bar][buz] to get the value of buz.
	 *
	 * Warning: free() after use.
	 *
	 */
	if (buf == NULL || key == NULL) {
		return NULL;
	}
	char *keybuf = malloc(strlen(key));
	char *tmp = format_json(buf);
	char *ret = NULL;
	for (size_t i = 0; i < strlen(key); i++) {
		if (key[i] == '[') {
			for (size_t j = i + 1; j < strlen(key); j++) {
				if (key[j] == ']') {
					ret = json_get_key_one_level(tmp, keybuf);
					if (ret == NULL) {
						free(keybuf);
						free(tmp);
						return NULL;
					}
					free(tmp);
					tmp = ret;
					break;
				}
				keybuf[j - i - 1] = key[j];
				keybuf[j - i] = '\0';
			}
		}
	}
	free(keybuf);
	tmp = ret;
	if (tmp == NULL) {
		return NULL;
	}
	ret = correct_backslash(tmp);
	free(tmp);
	return ret;
}
size_t json_anon_layer_get_key_array(const char *_Nonnull buf, const char *_Nonnull key, char ***_Nullable array)
{
	/*
	 * Warning: free() after use.
	 * Warning: **array should be NULL, it will be malloc()ed.
	 * From json anonymous layers, get all values of key.
	 * Return: The lenth we get.
	 *
	 * Example json:
	 * {{"foo":"bar"},{"foo":"buz"}}
	 * We use key [foo] to get the value of foo.
	 * So we return ["bar", "buz"].
	 *
	 * It allows to use [foo][bar] to get the value of bar,
	 * because it calls json_get_key() directly.
	 *
	 */
	if (buf == NULL || key == NULL) {
		return 0;
	}
	char *tmp = format_json(buf);
	(*array) = malloc(sizeof(char *));
	(*array)[0] = NULL;
	size_t ret = 0;
	const char *p = tmp;
	while (p != NULL) {
		(*array)[ret] = json_get_key(p, key);
		if ((*array)[ret] != NULL) {
			ret++;
			(*array) = realloc((*array), sizeof(char *) * (ret + 1));
			(*array)[ret] = NULL;
		}
		p = next_layer(p);
	}
	free(tmp);
	log("{base}ret: {cyan}%ld{clear}\n", ret);
	return ret;
}
size_t json_anon_layer_get_key_array_allow_null_val(const char *_Nonnull buf, const char *_Nonnull key, char ***_Nullable array)
{
	/*
	 * Warning: free() after use.
	 * Warning: **array should be NULL, it will be malloc()ed.
	 * From json anonymous layers, get all values of key.
	 * Return: The lenth we get.
	 *
	 * Example json:
	 * {{"foo":"bar"},{"foo":"buz"}}
	 * We use key [foo] to get the value of foo.
	 * So we return ["bar", "buz"].
	 *
	 * It allows to use [foo][bar] to get the value of bar,
	 * because it calls json_get_key() directly.
	 *
	 */
	if (buf == NULL || key == NULL) {
		return 0;
	}
	char *tmp = format_json(buf);
	(*array) = malloc(sizeof(char *));
	(*array)[0] = NULL;
	size_t ret = 0;
	const char *p = tmp;
	while (p != NULL) {
		(*array)[ret] = json_get_key(p, key);
		ret++;
		(*array) = realloc((*array), sizeof(char *) * (ret + 1));
		(*array)[ret] = NULL;
		p = next_layer(p);
	}
	free(tmp);
	log("{base}ret: {cyan}%ld{clear}\n", ret);
	return ret;
}
char *json_anon_layer_get_key(const char *_Nonnull buf, const char *_Nonnull key, const char *_Nonnull value, const char *_Nonnull key_to_get)
{
	/*
	 * Warning: free() after use.
	 * From json anonymous layers, get key_to_get in the layer that key==value.
	 * Return: The value we get.
	 *
	 * Example json:
	 * {{"foo":"xxx",bar:"yyy"},{"foo":"buz","bar":"xxx"}}
	 * We use key [foo] and value buz to get the value of bar.
	 * So we return xxx.
	 *
	 * It will call json_get_key() directly.
	 *
	 */
	const char *p = buf;
	char *valtmp = NULL;
	while (p != NULL) {
		valtmp = json_get_key(p, key);
		if (valtmp == NULL) {
			p = next_layer(p);
			continue;
		}
		if (strcmp(valtmp, value) == 0) {
			char *ret = json_get_key(p, key_to_get);
			free(valtmp);
			return ret;
		}
		free(valtmp);
		p = next_layer(p);
	}
	return NULL;
}
char *json_open_file(const char *_Nonnull path)
{
	/*
	 * Warning: free() after use.
	 *
	 * Open a json file and return the content.
	 * It will return NULL if failed.
	 * Just a simple wrapper of open() and read().
	 *
	 */
	struct stat st;
	if (stat(path, &st) == -1) {
		return 0;
	}
	char *ret = malloc((size_t)st.st_size + 3);
	int fd = open(path, O_RDONLY | O_CLOEXEC);
	if (fd == -1) {
		return NULL;
	}
	ssize_t size = read(fd, ret, (size_t)st.st_size);
	ret[size] = '\0';
	return ret;
}
