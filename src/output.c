/*
 * music player command (mpc)
 * Copyright (C) 2003-2017 The Music Player Daemon Project
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

#include "output.h"
#include "charset.h"
#include "options.h"
#include "util.h"
#include "args.h"
#include "search.h"
#include "status.h"
#include "path.h"
#include "Compiler.h"

#include <mpd/client.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int
cmd_outputs(gcc_unused int argc, gcc_unused char **argv,
	    struct mpd_connection *conn)
{
	mpd_send_outputs(conn);

	struct mpd_output *output;
	while ((output = mpd_recv_output(conn)) != NULL) {
		/* We increment by 1 to make it natural to the user  */
		int id = mpd_output_get_id(output) + 1;
		const char *name = mpd_output_get_name(output);

		if (mpd_output_get_enabled(output)) {
			printf("Output %i (%s) is enabled\n", id, name);
		} else {
			printf("Output %i (%s) is disabled\n", id, name);
		}

		mpd_output_free(output);
	}

	my_finishCommand(conn);
	return 0;
}

static unsigned
match_outputs(struct mpd_connection *conn,
	      char **names, char **names_end, unsigned **ids_end)
{
	unsigned max = 0, *id = *ids_end;

	mpd_send_outputs(conn);

	struct mpd_output *output;
	while ((output = mpd_recv_output(conn)) != NULL) {
		const char *name = mpd_output_get_name(output);
		max = mpd_output_get_id(output);

		for (char **n = names; n != names_end; ++n) {
			if (!strcmp(*n, name)) {
				*id = max;
				++id;
				*n = *names;
				++names;
				break;
			}
		}

		mpd_output_free(output);
	}

	my_finishCommand(conn);

	for (char **n = names; n != names_end; ++n) {
		fprintf(stderr, "%s: no such output\n", *n);
	}

	*ids_end = id;
	return max;
}

static int
enable_disable(int argc, char **argv, struct mpd_connection *conn,
	       bool (*matched)(struct mpd_connection *conn, unsigned id),
	       bool (*not_matched)(struct mpd_connection *conn, unsigned id))
{
	char **names = argv, **names_end = argv;

	bool only = false;
	if (not_matched != NULL && !strcmp(argv[0], "only")) {
		only = true;
		++argv;
		if (!--argc) {
			DIE("No outputs specified.");
		}
	}

	unsigned *ids = malloc(argc * sizeof *ids);
	unsigned *ids_end = ids;

	for (int i = argc; i; --i, ++argv) {
		int arg;
		if (!parse_int(*argv, &arg)) {
			*names_end = *argv;
			++names_end;
		} else if (arg <= 0) {
			fprintf(stderr, "%s: not a positive integer\n", *argv);
		} else {
			/* We decrement by 1 to make it natural to the user. */
			*ids_end++ = arg - 1;
		}
	}

	unsigned max;
	if (only || names != names_end) {
		max = match_outputs(conn, names, names_end, &ids_end);
	}

	if (ids == ids_end) {
		goto done;
	}

	if (!mpd_command_list_begin(conn, false)) {
		printErrorAndExit(conn);
	}

	if (only) {
		for (unsigned i = 0; i <= max; ++i) {
			bool found = false;
			for (unsigned *id = ids;
			     !found && id != ids_end;
			     ++id) {
				found = *id == i;
			}
			(found ? matched : not_matched)(conn, i);
		}
	} else {
		for (unsigned *id = ids; id != ids_end; ++id) {
			matched(conn, *id);
		}
	}

	if (!mpd_command_list_end(conn) || !mpd_response_finish(conn)) {
		printErrorAndExit(conn);
	}

	cmd_outputs(0, NULL, conn);

done:
	free(ids);
	return 0;
}

int
cmd_enable(int argc, char **argv, struct mpd_connection *conn)
{
	return enable_disable(argc, argv, conn, mpd_send_enable_output,
			      mpd_send_disable_output);
}

int
cmd_disable(gcc_unused int argc, char **argv, struct mpd_connection *conn)
{
	return enable_disable(argc, argv, conn, mpd_send_disable_output,
			      mpd_send_enable_output);
}

int
cmd_toggle_output(int argc, char **argv, struct mpd_connection *conn)
{
	return enable_disable(argc, argv, conn, mpd_send_toggle_output,
			      mpd_send_toggle_output);
}
