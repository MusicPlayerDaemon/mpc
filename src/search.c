/*
 * Copyright (C) 2008-2010 The Music Player Daemon Project
 * http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "search.h"
#include "util.h"
#include "charset.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define DIE(...) do { fprintf(stderr, __VA_ARGS__); return false; } while(0)

enum mpd_tag_type
get_search_type(const char *name)
{
	enum mpd_tag_type type;

	if (strcasecmp(name, "any") == 0)
		return SEARCH_TAG_ANY;

	if (strcasecmp(name, "filename") == 0)
		return SEARCH_TAG_URI;

	type = mpd_tag_name_iparse(name);
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

int
get_constraints(int argc, char **argv, struct constraint **constraints)
{
	int numconstraints = 0;
	int type;
	int i;

	assert(argc > 0 && argc % 2 == 0);

	*constraints = malloc(sizeof(**constraints) * argc / 2);

	for (i = 0; i < argc; i += 2) {
		type = get_search_type(argv[i]);
		if (type < 0) {
			free(*constraints);
			return -1;
		}

		(*constraints)[numconstraints].type = type;
		(*constraints)[numconstraints].query = argv[i+1];
		numconstraints++;
	}

	return numconstraints;
}

static void my_finishCommand(struct mpd_connection *conn) {
	if (!mpd_response_finish(conn))
		printErrorAndExit(conn);
}

static void
add_constraint(struct mpd_connection *conn,
	       const struct constraint *constraint)
{
	if (constraint->type == (enum mpd_tag_type)SEARCH_TAG_ANY)
		mpd_search_add_any_tag_constraint(conn, MPD_OPERATOR_DEFAULT,
						  charset_to_utf8(constraint->query));
	else if (constraint->type == (enum mpd_tag_type)SEARCH_TAG_URI)
		mpd_search_add_any_tag_constraint(conn, MPD_OPERATOR_DEFAULT,
						  charset_to_utf8(constraint->query));
	else
		mpd_search_add_tag_constraint(conn, MPD_OPERATOR_DEFAULT,
					      constraint->type,
					      charset_to_utf8(constraint->query));
}

bool
add_constraints(int argc, char ** argv, struct mpd_connection *conn)
{
	struct constraint *constraints;
	int numconstraints;
	int i;

	if (argc % 2 != 0)
		DIE("arguments must be pairs of search types and queries\n");

	numconstraints = get_constraints(argc, argv, &constraints);
	if (numconstraints < 0)
		return false;

	for (i = 0; i < numconstraints; i++) {
		add_constraint(conn, &constraints[i]);
	}

	free(constraints);
	return true;
}

static int do_search ( int argc, char ** argv, struct mpd_connection *conn, int exact )
{
	mpd_search_db_songs(conn, exact);
	if (!add_constraints(argc, argv, conn))
		return -1;

	if (!mpd_search_commit(conn))
		printErrorAndExit(conn);

	print_filenames(conn);

	my_finishCommand(conn);

	return 0;
}

int
cmd_search(int argc, char **argv, struct mpd_connection *conn)
{
	return do_search(argc, argv, conn, 0);
}

int
cmd_find(int argc, char **argv, struct mpd_connection *conn)
{
	return do_search(argc, argv, conn, 1);
}

int
cmd_findadd(int argc, char **argv, struct mpd_connection *conn)
{
	if (mpd_connection_cmp_server_version(conn, 0, 16, 0) < 0)
		fprintf(stderr, "warning: MPD 0.16 required for this command\n");

	mpd_search_add_db_songs(conn, true);
	if (!add_constraints(argc, argv, conn))
		return -1;

	if (!mpd_search_commit(conn))
		printErrorAndExit(conn);

	my_finishCommand(conn);
	return 0;
}
