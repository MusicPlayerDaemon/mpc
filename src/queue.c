// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "queue.h"
#include "tags.h"
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

	const unsigned plLength = mpd_status_get_queue_length(status);

	bool *songsToDel = malloc(plLength);
	memset(songsToDel, false, plLength);

	for (unsigned i = 0; i < (unsigned)argc; ++i) {
		const char *const s = argv[i];

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
		} else if (*t=='\0')
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

		if ((unsigned)range[1] > plLength)
			DIE("song number does not exist: %i\n",range[1]);

		memset(songsToDel + range[0] - 1, true, range[1] - range[0] + 1);
	}

	if (!mpd_command_list_begin(conn, false))
		printErrorAndExit(conn);

	int songsDeleted = 0;
	for (unsigned i = 0; i < plLength; ++i) {
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
	/* ask MPD to omit the tags which are not used by the
	   `--format` to reduce network transfer for tag values we're
	   not going to use anyway */
	if (!mpd_command_list_begin(conn, false) ||
	    !send_tag_types_for_format(conn, options.format))
		printErrorAndExit(conn);

	bool ret = argc > 0
		? mpd_send_list_playlist_meta(conn, argv[0])
		: mpd_send_list_queue_meta(conn);

	if (ret == false)
		printErrorAndExit(conn);

	if (!mpd_command_list_end(conn))
		printErrorAndExit(conn);

	print_entity_list(conn, MPD_ENTITY_TYPE_SONG, true);
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

int cmd_insert (int argc, char ** argv, struct mpd_connection *conn )
{
	struct mpd_status *status = getStatus(conn);
	const unsigned from = mpd_status_get_queue_length(status);
	const int cur_pos = mpd_status_get_song_pos(status);
	const int next_id = mpd_status_get_next_song_id(status);
	const bool random_mode = mpd_status_get_random(status);
	mpd_status_free(status);

	int ret = cmd_add(argc, argv, conn);
	if (ret != 0)
		return ret;

	/* check the new queue length to find out how many songs were
	   appended  */
	const unsigned end = query_queue_length(conn);

	if (random_mode) {
		queue_range(conn, from, end, next_id);
		return 0;
	}

	if (end == from)
		return 0;

	/* move those songs to right after the current one */
	if (!mpd_run_move_range(conn, from, end, cur_pos + 1))
		printErrorAndExit(conn);

	return 0;
}


int
cmd_prio(int argc, char **argv, struct mpd_connection *conn)
{
	char *endptr;
	int i = 0;
	const char *s = argv[i++];
	int prio = strtol(s, &endptr, 10);
	if (endptr == s || *endptr != 0)
		DIE("Failed to parse number: %s\n", s);
	if (prio < 0 || prio > 255)
		DIE("Priority must be between 0 and 255: %s\n", s);

	if (!mpd_command_list_begin(conn, false))
		printErrorAndExit(conn);

	while (i < argc) {
		s = argv[i++];
		int position = strtol(s, &endptr, 10);
		if (endptr == s || *endptr != 0)
			DIE("Failed to parse number: %s\n", s);
		if (position < 1)
			DIE("Invalid song position: %s\n", s);

		/* mpc's song positions are 1-based, but MPD uses
		   0-based positions */
		--position;

		if (!mpd_send_prio(conn, prio, position))
			break;
	}

	mpd_command_list_end(conn);
	my_finishCommand(conn);
	return 0;
}
