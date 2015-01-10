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

#include "path.h"

#include <mpd/client.h>

#include <string.h>

static char *music_directory;
static bool initialized;

bool
path_prepare(struct mpd_connection *conn)
{
	if (initialized)
		return true;

	initialized = true;

	if (mpd_connection_cmp_server_version(conn, 0, 17, 0) < 0)
		/* the "config" command was added in MPD 0.17.1 */
		return true;

	if (!mpd_send_command(conn, "config", NULL))
		return false;

	struct mpd_pair *pair = mpd_recv_pair_named(conn, "music_directory");
	if (pair != NULL) {
		music_directory = strdup(pair->value);
		mpd_return_pair(conn, pair);
	}

	return mpd_response_finish(conn) || mpd_connection_clear_error(conn);
}

const char *
to_relative_path(const char *path)
{
	if (music_directory == NULL || path[0] != '/')
		return NULL;

	const size_t path_length = strlen(path);
	const size_t base_length = strlen(music_directory);
	if (base_length >= path_length ||
	    memcmp(music_directory, path, base_length) != 0)
		return NULL;

	if (path[base_length] == 0)
		return "/";

	if (path[base_length] == '/')
		return path + base_length + 1;

	return NULL;
}

