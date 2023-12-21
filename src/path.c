// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

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

