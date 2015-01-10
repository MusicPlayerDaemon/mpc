/*
 * music player command (mpc)
 * Copyright (C) 2003-2015 The Music Player Daemon Project
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

#include "idle.h"
#include "util.h"

#include <mpd/client.h>

#include <stdbool.h>
#include <stdio.h>

int cmd_idle(int argc, char **argv,
	     struct mpd_connection *connection)
{
	enum mpd_idle idle = 0;

	for (int i = 0; i < argc; ++i) {
		enum mpd_idle parsed = mpd_idle_name_parse(argv[i]);
		if (parsed == 0) {
			fprintf(stderr, "Unrecognized idle event: %s\n",
				argv[i]);
			return 1;
		}

		idle |= parsed;
	}

	idle = idle == 0 ? mpd_run_idle(connection)
		: mpd_run_idle_mask(connection, idle);
	if (idle == 0 &&
	    mpd_connection_get_error(connection) != MPD_ERROR_SUCCESS)
		printErrorAndExit(connection);

	for (unsigned j = 0;; ++j) {
		enum mpd_idle i = 1 << j;
		const char *name = mpd_idle_name(i);

		if (name == NULL)
			break;

		if (idle & i)
			printf("%s\n", name);
	}

	return 0;
}

int
cmd_idleloop(int argc, char **argv, struct mpd_connection *connection)
{
	while (true) {
		int ret = cmd_idle(argc, argv, connection);
		fflush(stdout);
		if (ret != 0)
			return ret;
	}
}
