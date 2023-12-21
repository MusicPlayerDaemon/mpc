// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "args.h"
#include "charset.h"
#include "list.h"
#include "options.h"
#include "strcasecmp.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#ifdef __unix__
#include <sys/param.h>
#endif

int
stdinToArgArray(char ***array)
{
	struct List list;
	makeList(&list);

	char buffer[4096];

	while (fgets(buffer, sizeof(buffer), stdin)) {
		char *sp;
		if((sp = strchr(buffer,'\n'))) *sp = '\0';
		insertInListWithoutKey(&list, strdup(buffer));
	}

	const unsigned size = list.numberOfNodes;
	*array = malloc((sizeof(char *))*size);
	unsigned i = 0;
	struct ListNode *node = list.firstNode;
	while(node) {
		(*array)[i++] = (char *)node->data;
		node = node->nextNode;
	}
	assert(i==size);

	freeList(&list);

	return size;
}

int
stdinAndPreambleToArgArray(char ***array, char *preamble)
{
	struct List list;
	makeList(&list);

	insertInListWithoutKey(&list, strdup(preamble));
	/* todo : factorize next statements... */

	char buffer[4096];

	while (fgets(buffer, sizeof(buffer), stdin)) {
		char *sp;
		if((sp = strchr(buffer,'\n'))) *sp = '\0';
		insertInListWithoutKey(&list, strdup(buffer));
	}

	const unsigned size = list.numberOfNodes;
	*array = malloc((sizeof(char *))*size);
	unsigned i = 0;
	struct ListNode *node = list.firstNode;
	while(node) {
		(*array)[i++] = (char *)node->data;
		node = node->nextNode;
	}
	assert(i==size);

	freeList(&list);

	return size;
}


void
free_pipe_array(unsigned max, char ** array)
{
	for (unsigned i = 0 ; i < max; ++i)
		free(array[i]);
}

bool
contains_absolute_path(unsigned argc, char **argv)
{
	for (unsigned i = 0; i < argc; ++i)
		if (argv[i][0] == '/')
			return true;

	return false;
}

bool
contains_absolute_path_from(unsigned argc, char **argv, unsigned from)
{
	for (unsigned i = from; i < argc; ++i)
		if (argv[i][0] == '/')
			return true;

	return false;
}

gcc_pure
static bool
uri_has_scheme(const char *uri)
{
	return strstr(uri, "://") != NULL;
}

void
strip_trailing_slash(char *s)
{
	if (uri_has_scheme(s))
		/* strip slashes only if this string is relative to
		   the music directory; absolute URLs are not, for
		   sure */
		return;

	size_t len = strlen(s);

	if (len == 0)
		return;
	--len;

	if (s[len] == '/')
		s[len] = '\0';

	return;
}

int
get_boolean(const char *arg)
{
	static const struct _bool_table {
		const char * on;
		const char * off;
	} bool_table [] = {
		{ "on", "off" },
		{ "1", "0" },
		{ "true", "false" },
		{ "yes", "no" },
		{ .on = NULL }
	};

	for (unsigned i = 0; bool_table[i].on != NULL; ++i) {
		if (strcasecmp(arg,bool_table[i].on) == 0)
			return 1;
		else if (strcasecmp(arg,bool_table[i].off) == 0)
			return 0;
	}

	fprintf(stderr,"\"%s\" is not a boolean value: <",arg);

	for (unsigned i = 0; bool_table[i].on != NULL; ++i) {
		fprintf(stderr,"%s|%s%s", bool_table[i].on,
			bool_table[i].off,
			( bool_table[i+1].off ? "|" : ">\n"));
	}

	return -1;
}

bool
parse_int(const char *str, int *ret)
{
	char *test;
	int temp = strtol(str, &test, 10);

	if (*test != '\0')
		return false; /* failure */

	*ret = temp;
	return true; /* success */
}

bool
parse_unsigned(const char *str, unsigned *ret)
{
	char *test;
	unsigned temp = strtoul(str, &test, 10);

	if (*test != '\0')
		return false; /* failure */

	*ret = temp;
	return true; /* success */
}

bool
parse_float(const char *str, float *ret)
{
	char *test;
	float temp = strtof(str, &test);

	if (*test != '\0')
		return false; /* failure */

	*ret = temp;
	return true; /* success */
}

bool
parse_songnum(const char *str, int *ret)
{
	if (!str)
		return false;
	if (*str == '#')
		str++;

	char *endptr;
	int song = strtol(str, &endptr, 10);

	if (str == endptr || (*endptr != ')' && *endptr != '\0') || song < 1)
		return false;

	*ret = song;
	return true;
}

bool
parse_int_value_change(const char *str, struct int_value_change *ret)
{
	const size_t len = strlen(str);
	if (len < 1)
		return false;

	int relative = 0;
	if (*str == '+')
		relative = 1;
	else if (*str == '-')
		relative = -1;

	int change;
	if (!parse_int(str, &change))
		return false;

	ret->value = change;
	ret->is_relative = (relative != 0);
	return true;
}
