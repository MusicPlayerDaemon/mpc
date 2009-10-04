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

#include "idle.h"
#include "util.h"

#include <mpd/client.h>

#include <stdbool.h>
#include <stdio.h>

int cmd_idle(mpd_unused int argc, mpd_unused char **argv,
	     struct mpd_connection *connection)
{
	enum mpd_idle idle;

	idle = mpd_run_idle(connection);
	if (idle == 0)
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
	int ret;

	while (true) {
		ret = cmd_idle(argc, argv, connection);
		if (ret != 0)
			return ret;
	}
}
