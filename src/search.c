/*
 * Copyright (C) 2003-2009 The Music Player Daemon Project
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

#include <mpd/client.h>

#include <stdio.h>
#include <stdlib.h>

#define DIE(...) do { fprintf(stderr, __VA_ARGS__); return -1; } while(0)

static void my_finishCommand(struct mpd_connection *conn) {
	if (!mpd_response_finish(conn))
		printErrorAndExit(conn);
}

static int
add_constraints(int argc, char ** argv, struct mpd_connection *conn)
{
	Constraint *constraints;
	int numconstraints;
	int i;

	if (argc % 2 != 0)
		DIE("arguments must be pairs of search types and queries\n");

	numconstraints = get_constraints(argc, argv, &constraints);
	if (numconstraints < 0)
		return -1;

	for (i = 0; i < numconstraints; i++) {
		mpd_search_add_tag_constraint(conn, MPD_OPERATOR_DEFAULT,
					      constraints[i].type,
					      charset_to_utf8(constraints[i].query));
	}

	free(constraints);
	return 0;
}

static int do_search ( int argc, char ** argv, struct mpd_connection *conn, int exact )
{
	int ret;

	mpd_search_db_songs(conn, exact);
	ret = add_constraints(argc, argv, conn);
	if (ret != 0)
		return ret;

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
	int ret;

	if (mpd_connection_cmp_server_version(conn, 0, 16, 0) < 0)
		fprintf(stderr, "warning: MPD 0.16 required for this command\n");

	mpd_search_add_db_songs(conn, true);
	ret = add_constraints(argc, argv, conn);
	if (ret != 0)
		return ret;

	if (!mpd_search_commit(conn))
		printErrorAndExit(conn);

	my_finishCommand(conn);
	return 0;
}
