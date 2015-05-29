/* music player command (mpc)
 * Copyright (C) 2003-2008 Warren Dukes <warren.dukes@gmail.com>,
				Eric Wong <normalperson@yhbt.net>,
				Daniel Brown <danb@cs.utexas.edu>
 * Copyright (C) 2008-2010 Max Kellermann <max@duempel.org>
 * Project homepage: http://musicpd.org

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "queue.h"
#include "args.h"
#include "charset.h"
#include "options.h"
#include "util.h"
#include "path.h"
#include "Compiler.h"

#include <mpd/client.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SIMPLE_CMD(cmd_clear, mpd_run_clear, 1)
SIMPLE_CMD(cmd_shuffle, mpd_run_shuffle, 1)

int
cmd_add(int argc, char **argv, struct mpd_connection *conn)
{
	if (contains_absolute_path(argc, argv) && !path_prepare(conn))
		printErrorAndExit(conn);

	if (!mpd_command_list_begin(conn, false))
		printErrorAndExit(conn);

	for (int i = 0; i < argc; ++i) {
		strip_trailing_slash(argv[i]);

		const char *path = argv[i];
		const char *relative_path = to_relative_path(path);
		if (relative_path != NULL)
			path = relative_path;

		if (options.verbosity >= V_VERBOSE)
			printf("adding: %s\n", path);
		mpd_send_add(conn, charset_to_utf8(path));
	}

	if (!mpd_command_list_end(conn))
		printErrorAndExit(conn);

	if (!mpd_response_finish(conn)) {
#if LIBMPDCLIENT_CHECK_VERSION(2,4,0)
		if (mpd_connection_get_error(conn) == MPD_ERROR_SERVER) {
			/* check which of the arguments has failed */
			unsigned location =
				mpd_connection_get_server_error_location(conn);
			if (location < (unsigned)argc) {
				/* we've got a valid location from the
				   server */
				const char *message =
					mpd_connection_get_error_message(conn);
				message = charset_from_utf8(message);
				fprintf(stderr, "error adding %s: %s\n",
					argv[location], message);
				exit(EXIT_FAILURE);
			}
		}
#endif

		printErrorAndExit(conn);
	}

	return 0;
}

int
cmd_crop(gcc_unused int argc, gcc_unused char **argv,
	 struct mpd_connection *conn)
{
	struct mpd_status *status = getStatus(conn);
	int length = mpd_status_get_queue_length(status) - 1;

	if (length < 0) {
		mpd_status_free(status);
		DIE("A playlist longer than 1 song in length is required to crop.\n");
	} else if (mpd_status_get_state(status) == MPD_STATE_PLAY ||
		   mpd_status_get_state(status) == MPD_STATE_PAUSE) {
		if (!mpd_command_list_begin(conn, false))
			printErrorAndExit(conn);

		for (; length >= 0; --length)
			if (length != mpd_status_get_song_pos(status))
				mpd_send_delete(conn, length);

		mpd_status_free(status);

		mpd_command_list_end(conn);
		my_finishCommand(conn);
		return 0;
	} else {
		mpd_status_free(status);
		DIE("You need to be playing to crop the playlist\n");
	}
}

int
cmd_del(int argc, char **argv, struct mpd_connection *conn)
{
	struct mpd_status *status = getStatus(conn);

	const int plLength = mpd_status_get_queue_length(status);

	char *songsToDel = malloc(plLength);
	memset(songsToDel,0,plLength);

	for (int i = 0; i < argc; ++i) {
		char *s;
		if (argv[i][0]=='#')
			s = &argv[i][1];
		else
			s = argv[i];

		char *t;
		int range[2];
		range[0] = strtol(s, &t, 10);

		/* If argument is 0 current song and we're not stopped */
		if (range[0] == 0 && strlen(s) == 1 &&
		    (mpd_status_get_state(status) == MPD_STATE_PLAY ||
		     mpd_status_get_state(status) == MPD_STATE_PAUSE))
			range[0] = mpd_status_get_song_pos(status) + 1;

		if (s==t)
			DIE("error parsing song numbers from: %s\n", argv[i]);
		else if (*t=='-') {
			char *t2;
			range[1] = strtol(t+1, &t2, 10);
			if(t + 1 == t2 || *t2!='\0')
				DIE("error parsing range from: %s\n", argv[i]);
		} else if (*t == ')' || *t=='\0')
			range[1] = range[0];
		else
			DIE("error parsing song numbers from: %s\n", argv[i]);

		if (range[0] <= 0 || range[1] <= 0) {
			if (range[0] == range[1])
				DIE("song number must be positive: %i\n",
				    range[0]);
			else
				DIE("song numbers must be positive: %i to %i\n",
				    range[0], range[1]);
		}

		if (range[1] < range[0])
			DIE("song range must be from low to high: %i to %i\n",range[0],range[1]);

		if (range[1] > plLength)
			DIE("song number does not exist: %i\n",range[1]);

		for (int j = range[0]; j <= range[1]; j++)
			songsToDel[j - 1] = 1;
	}

	if (!mpd_command_list_begin(conn, false))
		printErrorAndExit(conn);

	int songsDeleted = 0;
	for (int i = 0; i < plLength; ++i) {
		if (songsToDel[i]) {
			mpd_send_delete(conn, i - songsDeleted);
			++songsDeleted;
		}
	}

	mpd_status_free(status);
	free(songsToDel);

	mpd_command_list_end(conn);
	my_finishCommand(conn);
	return 0;
}

int
cmd_playlist(int argc, char **argv, struct mpd_connection *conn)
{
	bool ret = argc > 0
		? mpd_send_list_playlist_meta(conn, argv[0])
		: mpd_send_list_queue_meta(conn);

	if (ret == false)
		printErrorAndExit(conn);

	struct mpd_song *song;
	while ((song = mpd_recv_song(conn)) != NULL) {
		pretty_print_song(song);
		mpd_song_free(song);
		printf("\n");
	}

	my_finishCommand(conn);
	return 0;
}

gcc_pure
static unsigned
query_queue_length(struct mpd_connection *conn)
{
	struct mpd_status *status = getStatus(conn);
	const unsigned length = mpd_status_get_queue_length(status);
	mpd_status_free(status);
	return length;
}

#if LIBMPDCLIENT_CHECK_VERSION(2,8,0)

static void
queue_range(struct mpd_connection *conn, unsigned start, unsigned end,
	    int next_id)
{
	struct mpd_song *song = next_id >= 0
		? mpd_run_get_queue_song_id(conn, next_id)
		: mpd_run_current_song(conn);
	unsigned prio = 0;
	if (song != NULL) {
		prio = mpd_song_get_prio(song);
		mpd_song_free(song);
	}

	if (prio < 255)
		++prio;

	if (!mpd_run_prio_range(conn, prio, start, end))
		printErrorAndExit(conn);
}

#endif

int cmd_insert (int argc, char ** argv, struct mpd_connection *conn )
{
	struct mpd_status *status = getStatus(conn);
	const unsigned from = mpd_status_get_queue_length(status);
	const int cur_pos = mpd_status_get_song_pos(status);
#if LIBMPDCLIENT_CHECK_VERSION(2,8,0)
	const int next_id = mpd_status_get_next_song_id(status);
	const bool random_mode = mpd_status_get_random(status);
#endif
	mpd_status_free(status);

	int ret = cmd_add(argc, argv, conn);
	if (ret != 0)
		return ret;

	/* check the new queue length to find out how many songs were
	   appended  */
	const unsigned end = query_queue_length(conn);

#if LIBMPDCLIENT_CHECK_VERSION(2,8,0)
	if (random_mode) {
		queue_range(conn, from, end, next_id);
		return 0;
	}
#endif

	if (end == from)
		return 0;

	/* move those songs to right after the current one */
	if (!mpd_run_move_range(conn, from, end, cur_pos + 1))
		printErrorAndExit(conn);

	return 0;
}
