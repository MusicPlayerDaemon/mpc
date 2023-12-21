// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "tab.h"
#include "charset.h"
#include "util.h"
#include "Compiler.h"

#include <mpd/client.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char *
tab_base(const char *prefix)
{
	const char *slash = strrchr(prefix, '/');
	if (slash == NULL)
		return NULL;

	const size_t length = slash - prefix;
	char *p = malloc(length + 1);
	memcpy(p, prefix, length);
	p[length] = '\0';
	return p;
}

static void
tab_send_list(const char *prefix, struct mpd_connection *conn)
{
	char *base = tab_base(prefix);
	if (!mpd_send_list_meta(conn, base))
		printErrorAndExit(conn);
	free(base);
}

int
cmd_loadtab(gcc_unused int argc, char **argv, struct mpd_connection *conn)
{
	assert(argc == 1);

	const char *const prefix = charset_to_utf8(argv[0]);
	const size_t prefix_length = strlen(prefix);

	tab_send_list(prefix, conn);

	struct mpd_playlist *pl;
	while ((pl = mpd_recv_playlist(conn)) != NULL) {
		const char *path = mpd_playlist_get_path(pl);
		if (memcmp(path, prefix, prefix_length) == 0)
			printf("%s\n", charset_from_utf8(path));

		mpd_playlist_free(pl);
	}

	my_finishCommand(conn);
	return 0;
}

int
cmd_lstab(gcc_unused int argc, char **argv, struct mpd_connection *conn)
{
	assert(argc == 1);

	const char *const prefix = charset_to_utf8(argv[0]);
	const size_t prefix_length = strlen(prefix);

	tab_send_list(prefix, conn);

	struct mpd_directory *dir;
	while ((dir = mpd_recv_directory(conn)) != NULL) {
		const char *path = mpd_directory_get_path(dir);
		if (memcmp(path, prefix, prefix_length) == 0)
			printf("%s\n", charset_from_utf8(path));

		mpd_directory_free(dir);
	}

	my_finishCommand(conn);

	return 0;
}

int
cmd_tab(gcc_unused int argc, char ** argv, struct mpd_connection *conn)
{
	assert(argc == 1);

	const char *const prefix = charset_to_utf8(argv[0]);
	const size_t prefix_length = strlen(prefix);

	tab_send_list(prefix, conn);

	struct mpd_entity *entity;
	while ((entity = mpd_recv_entity(conn)) != NULL) {
		const char entitytype = mpd_entity_get_type(entity);
		if (entitytype == MPD_ENTITY_TYPE_DIRECTORY) {
			const char *path = mpd_directory_get_path(mpd_entity_get_directory(entity));
			if (memcmp(path, prefix, prefix_length) == 0)
				printf("%s/\n", charset_from_utf8(path));
		} else if (entitytype == MPD_ENTITY_TYPE_SONG) {
			const char *path = mpd_song_get_uri(mpd_entity_get_song(entity));
			if (memcmp(path, prefix, prefix_length) == 0)
				printf("%s\n", charset_from_utf8(path));
		}

		mpd_entity_free(entity);
	}

	my_finishCommand(conn);
	return 0;
}
