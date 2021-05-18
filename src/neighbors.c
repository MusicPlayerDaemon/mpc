/*
 * music player command (mpc)
 * Copyright 2003-2021 The Music Player Daemon Project
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

#include "neighbors.h"
#include "util.h"
#include "charset.h"
#include "Compiler.h"

#include <mpd/client.h>

#include <stdio.h>

int
cmd_listneighbors(gcc_unused int argc, gcc_unused char **argv,
		  struct mpd_connection *connection)
{
	if (!mpd_send_command(connection, "listneighbors", NULL))
		printErrorAndExit(connection);

	struct mpd_pair *pair;
	while ((pair = mpd_recv_pair_named(connection, "neighbor")) != NULL) {
		printf("%s\n", charset_from_utf8(pair->value));
		mpd_return_pair(connection, pair);
	}

	if (!mpd_response_finish(connection))
		printErrorAndExit(connection);

	return 0;
}
