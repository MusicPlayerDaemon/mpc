// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "search.h"
#include "options.h"
#include "strcasecmp.h"
#include "tags.h"
#include "util.h"
#include "charset.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	SEARCH_TAG_ANY = MPD_TAG_COUNT + 1,
	SEARCH_TAG_URI,
	SEARCH_TAG_BASE,
};

struct constraint {
	enum mpd_tag_type type;
	char *query;
};

static enum mpd_tag_type
get_search_type(const char *name)
{
	if (strcasecmp(name, "any") == 0)
		return (enum mpd_tag_type)SEARCH_TAG_ANY;

	if (strcasecmp(name, "filename") == 0)
		return (enum mpd_tag_type)SEARCH_TAG_URI;

	if (strcasecmp(name, "base") == 0)
		return (enum mpd_tag_type)SEARCH_TAG_BASE;

	enum mpd_tag_type type = mpd_tag_name_iparse(name);
	if (type != MPD_TAG_UNKNOWN)
		return type;

	fprintf(stderr, "\"%s\" is not a valid search type: <any", name);

	for (unsigned i = 0; i < MPD_TAG_COUNT; i++) {
		name = mpd_tag_name(i);
		if (name == NULL)
			continue;

		fputc('|', stderr);
		fputs(name, stderr);
	}

	fputs(">\n", stderr);

	return MPD_TAG_UNKNOWN;
}

static int
get_constraints(int argc, char **argv, struct constraint **constraints)
{
	int numconstraints = 0;

	assert(argc > 0 && argc % 2 == 0);

	*constraints = malloc(sizeof(**constraints) * argc / 2);

	for (int i = 0; i < argc; i += 2) {
		enum mpd_tag_type type = get_search_type(argv[i]);
		if (type == MPD_TAG_UNKNOWN) {
			free(*constraints);
			return -1;
		}

		(*constraints)[numconstraints].type = type;
		(*constraints)[numconstraints].query = argv[i+1];
		numconstraints++;
	}

	return numconstraints;
}

static void
add_constraint(struct mpd_connection *conn,
	       const struct constraint *constraint)
{
	if (constraint->type == (enum mpd_tag_type)SEARCH_TAG_ANY)
		mpd_search_add_any_tag_constraint(conn, MPD_OPERATOR_DEFAULT,
						  charset_to_utf8(constraint->query));
	else if (constraint->type == (enum mpd_tag_type)SEARCH_TAG_URI)
		mpd_search_add_uri_constraint(conn, MPD_OPERATOR_DEFAULT,
					      charset_to_utf8(constraint->query));
	else if (constraint->type == (enum mpd_tag_type)SEARCH_TAG_BASE)
		mpd_search_add_base_constraint(conn, MPD_OPERATOR_DEFAULT,
					       charset_to_utf8(constraint->query));
	else
		mpd_search_add_tag_constraint(conn, MPD_OPERATOR_DEFAULT,
					      constraint->type,
					      charset_to_utf8(constraint->query));
}

bool
add_constraints(int argc, char ** argv, struct mpd_connection *conn)
{
	if (argc == 1 && argv[0][0] == '(') {
		return mpd_search_add_expression(conn, argv[0]);
	}

	struct constraint *constraints;

	if (argc % 2 != 0)
		DIE("arguments must be pairs of search types and queries\n");

	int numconstraints = get_constraints(argc, argv, &constraints);
	if (numconstraints < 0)
		return false;

	for (int i = 0; i < numconstraints; i++) {
		add_constraint(conn, &constraints[i]);
	}

	free(constraints);
	return true;
}

static int
do_search(int argc, char ** argv, struct mpd_connection *conn, bool exact)
{
	/* ask MPD to omit the tags which are not used by the
	   `--format` to reduce network transfer for tag values we're
	   not going to use anyway */
	if (!mpd_command_list_begin(conn, false) ||
	    !send_tag_types_for_format(conn, options.custom_format ? options.format : NULL))
		printErrorAndExit(conn);

	mpd_search_db_songs(conn, exact);
	if (!add_constraints(argc, argv, conn))
		return -1;

	if (!mpd_search_commit(conn))
		printErrorAndExit(conn);

	if (!mpd_command_list_end(conn))
		printErrorAndExit(conn);

	print_entity_list(conn, MPD_ENTITY_TYPE_SONG, options.custom_format);

	my_finishCommand(conn);

	return 0;
}

static int
do_searchadd(int argc, char **argv, struct mpd_connection *conn, bool exact)
{
	mpd_search_add_db_songs(conn, exact);
	if (!add_constraints(argc, argv, conn))
		return -1;

	if (!mpd_search_commit(conn))
		printErrorAndExit(conn);

	my_finishCommand(conn);
	return 0;
}

int
cmd_search(int argc, char **argv, struct mpd_connection *conn)
{
	return do_search(argc, argv, conn, false);
}

int
cmd_searchadd(int argc, char **argv, struct mpd_connection *conn)
{
	return do_searchadd(argc, argv, conn, false);
}

int
cmd_find(int argc, char **argv, struct mpd_connection *conn)
{
	return do_search(argc, argv, conn, true);
}

int
cmd_findadd(int argc, char **argv, struct mpd_connection *conn)
{
	return do_searchadd(argc, argv, conn, true);
}

int
cmd_searchplaylist(int argc, char **argv, struct mpd_connection *conn)
{
	(void)argc; // silence warning about unused argument
	const char* playlist = argv[0];
	const char* expression = argv[1];

	/* ask MPD to omit the tags which are not used by the
		`--format` to reduce network transfer for tag values we're
		not going to use anyway */
	if (!mpd_command_list_begin(conn, false) ||
		!send_tag_types_for_format(conn, options.custom_format ? options.format : NULL))
		printErrorAndExit(conn);

	mpd_playlist_search_begin(conn, playlist, expression);

	if (!mpd_playlist_search_commit(conn))
		printErrorAndExit(conn);

	if (!mpd_command_list_end(conn))
		printErrorAndExit(conn);

	print_entity_list(conn, MPD_ENTITY_TYPE_SONG, options.custom_format);

	my_finishCommand(conn);
	return 0;
}
