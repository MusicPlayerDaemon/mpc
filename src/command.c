// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "command.h"
#include "charset.h"
#include "options.h"
#include "strcasecmp.h"
#include "util.h"
#include "args.h"
#include "search.h"
#include "status.h"
#include "status_format.h"
#include "tags.h"
#include "path.h"
#include "group.h"
#include "Compiler.h"

#include <mpd/client.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

SIMPLE_CMD(cmd_next, mpd_run_next, 1)
SIMPLE_CMD(cmd_prev, mpd_run_previous, 1)
SIMPLE_CMD(cmd_stop, mpd_run_stop, 1)
SIMPLE_CMD(cmd_clearerror, mpd_run_clearerror, 1)

SIMPLE_ONEARG_CMD(cmd_save, mpd_run_save, 0)
SIMPLE_ONEARG_CMD(cmd_rm, mpd_run_rm, 0)

/**
 * Returns the id of the current song, but only if it is really
 * playing (playing or paused).
 */
static int
get_active_song(const struct mpd_status *status)
{
	return mpd_status_get_state(status) == MPD_STATE_PLAY ||
		mpd_status_get_state(status) == MPD_STATE_PAUSE
		? mpd_status_get_song_id(status)
		: -1;
}

/**
 * Wait until the song changes or until playback is started/stopped.
 */
static void
wait_current(struct mpd_connection *c)
{
	struct mpd_status *status = getStatus(c);

	const int old_song = get_active_song(status);
	mpd_status_free(status);

	int new_song;
	do {
		enum mpd_idle idle = mpd_run_idle_mask(c, MPD_IDLE_PLAYER);
		if (idle == 0)
			printErrorAndExit(c);

		status = getStatus(c);
		new_song = get_active_song(status);
		mpd_status_free(status);
	} while (new_song == old_song);
}

int
cmd_current(gcc_unused int argc, gcc_unused char **argv,
	    struct mpd_connection *conn)
{
	if (options.wait)
		wait_current(conn);

	if (!mpd_command_list_begin(conn, true) ||
	    !mpd_send_status(conn) ||
	    !mpd_send_current_song(conn) ||
	    !mpd_command_list_end(conn))
		printErrorAndExit(conn);

	struct mpd_status *status = mpd_recv_status(conn);
	if (status == NULL)
		printErrorAndExit(conn);

	if (mpd_status_get_state(status) == MPD_STATE_PLAY ||
	    mpd_status_get_state(status) == MPD_STATE_PAUSE) {
		if (!mpd_response_next(conn))
			printErrorAndExit(conn);

		struct mpd_song *song = mpd_recv_song(conn);
		if (song != NULL) {
			pretty_print_song(song);
			printf("\n");

			mpd_song_free(song);
		}

		my_finishCommand(conn);
	}

	mpd_status_free(status);
	return 0;
}

int
cmd_queued(gcc_unused int argc, gcc_unused char **argv,
	    struct mpd_connection *conn)
{
	struct mpd_status *status = getStatus(conn);
	const int next_id = mpd_status_get_next_song_id(status);
	mpd_status_free(status);

	struct mpd_song *next = next_id >= 0
		? mpd_run_get_queue_song_id(conn, next_id)
		: NULL;
	if (next != NULL) {
		pretty_print_song(next);
		printf("\n");

		mpd_song_free(next);
	}

	return 0;
}

int
cmd_cdprev(gcc_unused int argc, gcc_unused char **argv,
	   struct mpd_connection *conn)
{
	struct mpd_status *status = getStatus(conn);

	/* go to previous track if mpd is playing first 3 seconds of
	   current track otherwise seek to beginning of current
	   track */
	if (mpd_status_get_elapsed_time(status) < 3) {
		cmd_prev(0, NULL, conn);
	} else {
		if (!mpd_run_seek_id(conn, mpd_status_get_song_id(status), 0))
			printErrorAndExit(conn);
	}

	return 1;
}

int
cmd_toggle(gcc_unused int argc, gcc_unused char **argv,
	   struct mpd_connection *conn)
{
	struct mpd_status *status = getStatus(conn);

	if (mpd_status_get_state(status) == MPD_STATE_PLAY) {
		cmd_pause(0, NULL, conn);
	} else {
		cmd_play(0, NULL, conn);
	}
	return 1;
}

int
cmd_play(int argc, char **argv, struct mpd_connection *conn)
{
	assert(argc < 2);

	bool success;
	if (argc > 0) {
		const char *string = argv[0];
		int song;
		if (!parse_songnum(string, &song))
			DIE("error parsing song numbers from: %s\n", string);

		song--;

		success = mpd_run_play_pos(conn, song);
	} else
		success = mpd_run_play(conn);

	if (!success)
		printErrorAndExit(conn);

	return 1;
}

static int
find_songname_id(struct mpd_connection *conn, int argc, char **argv)
{
	int res = -1;

	mpd_search_queue_songs(conn, false);

	if (argc == 1) {
		const char *pattern = charset_to_utf8(argv[0]);
		mpd_search_add_any_tag_constraint(conn, MPD_OPERATOR_DEFAULT,
						  pattern);
	} else {
		int n = add_constraints(argc, argv, conn);
		if (n < 0)
			return -2;
	}

	mpd_search_commit(conn);

	struct mpd_song *song = mpd_recv_song(conn);
	if (song != NULL) {
		res = mpd_song_get_id(song);

		mpd_song_free(song);
	}

	my_finishCommand(conn);

	return res;
}

int
cmd_searchplay(gcc_unused int argc, char **argv, struct mpd_connection *conn)
{
	int id = find_songname_id(conn, argc, argv);
	if (id == -2)
		/* add_constraints() has failed */
		return -1;
	if (id < 0)
		DIE("error: playlist contains no matching song\n");

	if (!mpd_run_play_id(conn, id))
		printErrorAndExit(conn);
	return 1;
}

int
cmd_seek_through(gcc_unused int argc, gcc_unused char **argv,
	 struct mpd_connection *conn)
{
	char * arg = argv[0];

	int seekchange;
	int rel = 0;

	/* Detect +/- if exists point to the next char */
	if (*arg == '+')
		rel = 1;
	else if (*arg == '-')
		rel = -1;

	if (rel == 0)
		rel = 1;
        else
		++arg;

	int total_secs;

	if (strchr(arg, ':') != NULL) {
		int hr = 0;
		int min = 0;
		int sec = 0;

		/* Take the seconds off the end of arg */
		char *sec_ptr = strrchr(arg, ':');

		/* Remove ':' and move the pointer one byte up */
		*sec_ptr = '\0';
		++sec_ptr;

		/* If hour is in the argument, else just point to the arg */
		char *min_ptr = strrchr(arg, ':');
		if (min_ptr != NULL) {

			/* Remove ':' and move the pointer one byte up */
			*min_ptr = '\0';
			++min_ptr;

			/* If the argument still exists, it's the hour  */
			if (arg != NULL) {
				char *hr_ptr = arg;
				char *test;
				hr = strtol(hr_ptr, &test, 10);

				if (*test != '\0' ||
					(rel == 1 && hr < 0))
					DIE("\"%s\" is not a positive number\n", sec_ptr);
			}
		} else {
			min_ptr = arg;
		}

		/* Change the pointers to a integer  */
		char *test;
		sec = strtol(sec_ptr, &test, 10);

		if (*test != '\0' || (rel == 1 && sec < 0))
			DIE("\"%s\" is not a positive number\n", sec_ptr);

		min = strtol( min_ptr, &test, 10 );

		if( *test != '\0' || (rel == 1 && min < 0 ))
			DIE("\"%s\" is not a positive number\n", min_ptr);

		/* If mins exist, check secs. If hrs exist, check mins  */
		if (min && strlen(sec_ptr) != 2)
			DIE("\"%s\" is not two digits\n", sec_ptr);
		else if (hr && strlen(min_ptr) != 2)
			DIE("\"%s\" is not two digits\n", min_ptr);

		/* Finally, make sure they're not above 60 if higher unit exists */
		if (min && sec > 60)
			DIE("\"%s\" is greater than 60\n", sec_ptr);
		else if (hr && min > 60 )
			DIE("\"%s\" is greater than 60\n", min_ptr);

		total_secs = (hr * 3600) + (min * 60) + sec;

	} else {

		/* absolute seek (in seconds) */
		char *test;
		total_secs = strtol(arg, &test, 10); /* get the # of seconds */

		if (*test != '\0' || (rel == 1 && total_secs < 0))
			DIE("\"%s\" is not a positive number\n", arg);
	}

	seekchange = total_secs;

	int k = 0;
	bool is_playing = 1;
	bool is_paused = 1;
	bool initial_is_paused = 0;
	while ((is_playing || is_paused) && k++ < 100) {
		struct mpd_status *status;
		status = getStatus(conn);
		int track_duration = mpd_status_get_total_time(status);
		int track_elapsed = mpd_status_get_elapsed_time(status);
		is_playing = mpd_status_get_state(status) == MPD_STATE_PLAY;
		is_paused = mpd_status_get_state(status) == MPD_STATE_PAUSE;
		int songpos = mpd_status_get_song_pos(status);
		mpd_status_free(status);
		if (k == 1)
			initial_is_paused = is_paused;

		if ((rel >= 0) && (seekchange >= track_duration - track_elapsed)) {
			seekchange -= track_duration - track_elapsed;
			if (!mpd_run_next(conn))
				printErrorAndExit(conn);
			/* checking if end of playlist has been reached */
			status = getStatus(conn);
			track_duration = mpd_status_get_total_time(status);
			is_playing = mpd_status_get_state(status) == MPD_STATE_PLAY;
			mpd_status_free(status);
			if (!is_playing)
				return -127;
		}

		if ((rel < 0) && (seekchange > track_elapsed)) {
			seekchange -= track_elapsed;
			if (!mpd_run_previous(conn))
				printErrorAndExit(conn);
			status = getStatus(conn);
			if (mpd_status_get_song_pos(status) == songpos) {
				seekchange = 0;
				break;
			}
			track_duration = mpd_status_get_total_time(status);
			mpd_status_free(status);
                        seekchange -= track_duration;
			if (seekchange < 0) {
				rel = 1;
				seekchange = -seekchange;
			}
		}
		if ((rel >= 0) && (seekchange < track_duration - track_elapsed))
			break;
		if ((rel < 0) && (seekchange <= track_elapsed))
			break;
	}
        if (initial_is_paused)
                cmd_pause(0, NULL, conn);

	if (seekchange == 0)
		return 1;

	char buffer[32];

	/* This detects +/- and is necessary due to the parsing of HH:MM:SS numbers*/
	if (rel < 0)
		seekchange = -seekchange;

	if (rel)
		snprintf(buffer, sizeof(buffer), "%+d", seekchange);
	else
		snprintf(buffer, sizeof(buffer), "%d", seekchange);

	if (!mpd_send_command(conn, "seekcur", buffer, NULL) ||
	    !mpd_response_finish(conn))
		printErrorAndExit(conn);

	return 1;
}

int
cmd_seek(gcc_unused int argc, gcc_unused char **argv,
	 struct mpd_connection *conn)
{
	char * arg = argv[0];

	int seekchange;
	int rel = 0;

	/* Detect +/- if exists point to the next char */
	if (*arg == '+')
		rel = 1;
	else if (*arg == '-')
		rel = -1;

	if (rel != 0)
		++arg;

	/* If seeking by percent */
	if (arg[strlen(arg) - 1] == '%') {
		/* Remove the % */
		arg[strlen(arg) - 1] = '\0';

		struct mpd_status *status;
		status = getStatus(conn);

		if (mpd_status_get_state(status) == MPD_STATE_STOP)
			DIE("not currently playing\n");

		/* percent seek, strtod is needed for percent with decimals */
		char *test;
		double perc = strtod(arg,&test);
		if (*test != '\0' || (rel == 0 && (perc < 0 || perc > 100)) ||
		    (rel && perc > abs(100)))
			DIE("\"%s\" is not an number between 0 and 100\n", arg);

		seekchange = perc * mpd_status_get_total_time(status) / 100 + 0.5;

		mpd_status_free(status);

	} else { /* If seeking by absolute seek time */

		int total_secs;

		if (strchr(arg, ':') != NULL) {
			int hr = 0;
			int min = 0;
			int sec = 0;

			/* Take the seconds off the end of arg */
			char *sec_ptr = strrchr(arg, ':');

			/* Remove ':' and move the pointer one byte up */
			*sec_ptr = '\0';
			++sec_ptr;

			/* If hour is in the argument, else just point to the arg */
			char *min_ptr = strrchr(arg, ':');
			if (min_ptr != NULL) {

				/* Remove ':' and move the pointer one byte up */
				*min_ptr = '\0';
				++min_ptr;

				/* If the argument still exists, it's the hour  */
				if (arg != NULL) {
					char *hr_ptr = arg;
					char *test;
					hr = strtol(hr_ptr, &test, 10);

					if (*test != '\0' ||
					    (rel == 0 && hr < 0))
						DIE("\"%s\" is not a positive number\n", sec_ptr);
				}
			} else {
				min_ptr = arg;
			}

			/* Change the pointers to a integer  */
			char *test;
			sec = strtol(sec_ptr, &test, 10);

			if (*test != '\0' || (rel == 0 && sec < 0))
				DIE("\"%s\" is not a positive number\n", sec_ptr);

			min = strtol( min_ptr, &test, 10 );

			if( *test != '\0' || (rel == 0 && min < 0 ))
				DIE("\"%s\" is not a positive number\n", min_ptr);

			/* If mins exist, check secs. If hrs exist, check mins  */
			if (min && strlen(sec_ptr) != 2)
				DIE("\"%s\" is not two digits\n", sec_ptr);
			else if (hr && strlen(min_ptr) != 2)
				DIE("\"%s\" is not two digits\n", min_ptr);

			/* Finally, make sure they're not above 60 if higher unit exists */
			if (min && sec > 60)
				DIE("\"%s\" is greater than 60\n", sec_ptr);
			else if (hr && min > 60 )
				DIE("\"%s\" is greater than 60\n", min_ptr);

			total_secs = (hr * 3600) + (min * 60) + sec;

		} else {

			/* absolute seek (in seconds) */
			char *test;
			total_secs = strtol(arg, &test, 10); /* get the # of seconds */

			if (*test != '\0' || (rel == 0 && total_secs < 0))
				DIE("\"%s\" is not a positive number\n", arg);
		}

		seekchange = total_secs;
	}

	char buffer[32];

	/* This detects +/- and is necessary due to the parsing of HH:MM:SS numbers*/
	if (rel < 0)
		seekchange = -seekchange;

	if (rel)
		snprintf(buffer, sizeof(buffer), "%+d", seekchange);
	else
		snprintf(buffer, sizeof(buffer), "%d", seekchange);

	if (!mpd_send_command(conn, "seekcur", buffer, NULL) ||
	    !mpd_response_finish(conn))
		printErrorAndExit(conn);

	return 1;
}

int
cmd_move(gcc_unused int argc, char **argv, struct mpd_connection *conn)
{
	int from;
	if (!parse_int(argv[0], &from) || from <= 0)
		DIE("\"%s\" is not a positive integer\n", argv[0]);

	int to;
	if (!parse_int(argv[1], &to) || to <= 0)
		DIE("\"%s\" is not a positive integer\n", argv[1]);

	/* users type in 1-based numbers, mpd uses 0-based */
	--from;
	--to;

	if (!mpd_run_move(conn, from, to))
		printErrorAndExit(conn);
	return 0;
}

int
cmd_moveplaylist(gcc_unused int argc, char **argv, struct mpd_connection *conn)
{
	const char* playlist = argv[0];

	int from;
	if (!parse_int(argv[1], &from) || from <= 0)
		DIE("\"%s\" is not a positive integer\n", argv[1]);

	int to;
	if (!parse_int(argv[2], &to) || to <= 0)
		DIE("\"%s\" is not a positive integer\n", argv[2]);

	/* users type in 1-based numbers, mpd uses 0-based */
	--from;
	--to;

	if (!mpd_run_playlist_move(conn, playlist, from, to))
		printErrorAndExit(conn);
	return 0;
}

int
cmd_listall(int argc, char **argv, struct mpd_connection *conn)
{
	const char * listall = "";
	int i = 0;

	if (argc > 0)
		listall = charset_to_utf8(argv[i]);

	do {
		char *tmp = strdup(listall);
		strip_trailing_slash(tmp);

		if (options.custom_format) {
			/* ask MPD to omit the tags which are not used
			   by the `--format` to reduce network
			   transfer for tag values we're not going to
			   use anyway */
			if (!mpd_command_list_begin(conn, false) ||
			    !send_tag_types_for_format(conn, options.format))
				printErrorAndExit(conn);

			if (!mpd_send_list_all_meta(conn, tmp))
				printErrorAndExit(conn);

			if (!mpd_command_list_end(conn))
				printErrorAndExit(conn);

			print_entity_list(conn, MPD_ENTITY_TYPE_SONG, true);
		} else {
			if (!mpd_send_list_all(conn, tmp))
				printErrorAndExit(conn);

			print_filenames(conn);
		}

		my_finishCommand(conn);
		free(tmp);
	} while (++i < argc && (listall = charset_to_utf8(argv[i])) != NULL);

	return 0;
}

static int
update_db(int argc, char **argv, struct mpd_connection *conn, bool rescan)
{
	if (contains_absolute_path(argc, argv) && !path_prepare(conn))
		printErrorAndExit(conn);

	if (!mpd_command_list_begin(conn, false))
		printErrorAndExit(conn);

	int i = 0;
	const char *update = "";
	if (argc > 0)
		update = charset_to_utf8(argv[i]);

	do {
		char *tmp = strdup(update);
		strip_trailing_slash(tmp);

		const char *path = tmp;
		const char *relative_path = to_relative_path(path);
		if (relative_path != NULL)
			path = relative_path;

		if (rescan)
			mpd_send_rescan(conn, path);
		else
			mpd_send_update(conn, path);

		free(tmp);
	} while (++i < argc && (update = charset_to_utf8(argv[i])) != NULL);

	if (!mpd_command_list_end(conn))
		printErrorAndExit(conn);

	/* obtain the last "update id" response */

	unsigned id = 0;
	while (true) {
		unsigned next_id = mpd_recv_update_id(conn);
		if (next_id == 0)
			break;
		id = next_id;
	}

	my_finishCommand(conn);

	while (options.wait) {
		/* idle until an update finishes */
		enum mpd_idle idle = mpd_run_idle_mask(conn, MPD_IDLE_UPDATE);
		struct mpd_status *status;
		unsigned current_id;

		if (idle == 0)
			printErrorAndExit(conn);

		/* determine the current "update id" */

		status = getStatus(conn);
		current_id = mpd_status_get_update_id(status);
		mpd_status_free(status);

		/* is our last queued update finished now? */

		if (current_id == 0 || current_id > id ||
		    (id > (1 << 30) && id < 1000)) /* wraparound */
			break;
	}

	return 1;
}

int
cmd_update(int argc, char **argv, struct mpd_connection *conn)
{
	return update_db(argc, argv, conn, false);
}

int
cmd_rescan(int argc, char **argv, struct mpd_connection *conn)
{
	return update_db(argc, argv, conn, true);
}

static int
ls_entity(int argc, char **argv, struct mpd_connection *conn,
	  enum mpd_entity_type type)
{
	const char *ls = "";
	int i = 0;
	if (argc > 0)
		ls = charset_to_utf8(argv[i]);

	/* ask MPD to omit the tags which are not used by the
	   `--format` to reduce network transfer for tag values we're
	   not going to use anyway */
	if (!mpd_command_list_begin(conn, false) ||
	    !send_tag_types_for_format(conn, options.custom_format ? options.format : NULL) ||
	    !mpd_command_list_end(conn))
		printErrorAndExit(conn);
	my_finishCommand(conn);

	do {
		if (!mpd_send_list_meta(conn, ls))
			printErrorAndExit(conn);

		print_entity_list(conn, type, options.custom_format);
		my_finishCommand(conn);
	} while (++i < argc && (ls = charset_to_utf8(argv[i])) != NULL);

	return 0;
}

int
cmd_ls(int argc, char **argv, struct mpd_connection *conn)
{
	for (int i = 0; i < argc; i++)
		strip_trailing_slash(argv[i]);

	return ls_entity(argc, argv, conn, MPD_ENTITY_TYPE_UNKNOWN);
}

int
cmd_lsplaylists(int argc, char **argv, struct mpd_connection *conn)
{
	return ls_entity(argc, argv, conn, MPD_ENTITY_TYPE_PLAYLIST);
}

int
cmd_lsdirs(int argc, char **argv, struct mpd_connection *conn)
{
	for (int i = 0; i < argc; i++)
		strip_trailing_slash(argv[i]);

	return ls_entity(argc, argv, conn, MPD_ENTITY_TYPE_DIRECTORY);
}

int
cmd_addplaylist(int argc, char **argv, struct mpd_connection *conn)
{
	const char* playlist = argv[0];

	if (contains_absolute_path_from(argc, argv, 1) && !path_prepare(conn))
		printErrorAndExit(conn);

	if (!mpd_command_list_begin(conn, false))
		printErrorAndExit(conn);

	for (int i = 1; i < argc; ++i) {
		strip_trailing_slash(argv[i]);

		const char *path = argv[i];
		const char *relative_path = to_relative_path(path);
		if (relative_path != NULL)
			path = relative_path;

		if (options.verbosity >= V_VERBOSE)
			printf("adding: %s\n", path);
		if (!mpd_send_playlist_add(conn, playlist, charset_to_utf8(path))) {
			printErrorAndExit(conn);
		}
	}

	mpd_command_list_end(conn);
	my_finishCommand(conn);

	return 0;
}

int
cmd_delplaylist(int argc, char **argv, struct mpd_connection *conn)
{
	const char* playlist = argv[0];

	if (!mpd_command_list_begin(conn, false))
		printErrorAndExit(conn);

	unsigned position;

	for (int i = 1; i < argc; ++i) {

		if (!parse_unsigned(argv[i], &position)) {
			DIE("Failed to parse unsigned number: %s\n", argv[i]);
		}

		if (options.verbosity >= V_VERBOSE)
			printf("del: %d\n", position);

		/* mpc's song positions are 1-based, but MPD uses
		   0-based positions */
		--position;

		if (!mpd_send_playlist_delete(conn, playlist, position)) {
			printErrorAndExit(conn);
		}
	}

	mpd_command_list_end(conn);
	my_finishCommand(conn);

	return 0;
}

int
cmd_renplaylist(int argc, char **argv, struct mpd_connection *conn)
{
	(void)argc; // silence warning about unused argument
	const char* playlist = argv[0];
	const char* newplaylist = argv[1];

	if (!mpd_run_rename(conn, playlist, newplaylist))
		printErrorAndExit(conn);

	return 0;
}

int
cmd_clearplaylist(int argc, char **argv, struct mpd_connection *conn)
{
	(void)argc; // silence warning about unused argument
	const char* playlist = argv[0];

	if (!mpd_run_playlist_clear(conn, playlist))
		printErrorAndExit(conn);

	return 0;
}

int
cmd_load(int argc, char **argv, struct mpd_connection *conn)
{
	const bool range = options.range.start > 0 ||
		options.range.end < UINT_MAX;

	if (!mpd_command_list_begin(conn, false))
		printErrorAndExit(conn);

	for (int i = 0; i < argc; ++i) {
		printf("loading: %s\n",argv[i]);

		const char *name_utf8 = charset_to_utf8(argv[i]);
		if (range)
			mpd_send_load_range(conn, name_utf8,
					    options.range.start,
					    options.range.end);
		else
			mpd_send_load(conn, name_utf8);
	}

	mpd_command_list_end(conn);
	my_finishCommand(conn);
	return 0;
}

int
cmd_tags(gcc_unused int argc, gcc_unused char **argv, gcc_unused struct mpd_connection *conn)
{
	const char *name = NULL;

	for (unsigned i = 0; i < MPD_TAG_COUNT; i++) {
		name = mpd_tag_name(i);
		if (name != NULL) {
			printf("%s\n", name);
		}
	}
	
	return 0;
}

int
cmd_list(int argc, char **argv, struct mpd_connection *conn)
{
	const char *name = argv[0];
	enum mpd_tag_type type = mpd_tag_name_iparse(name);
	if (type == MPD_TAG_UNKNOWN) {
		fprintf(stderr, "Unknown tag \"%s\"; supported tags are: ",
			name);

		bool first = true;
		for (unsigned i = 0; i < MPD_TAG_COUNT; i++) {
			name = mpd_tag_name(i);
			if (name == NULL)
				continue;

			if (first)
				first = false;
			else
				fputs(", ", stderr);
			fputs(name, stderr);
		}

		fputc('\n', stderr);

		return -1;
	}

	--argc;
	++argv;

	struct mpc_groups groups;
	mpc_groups_init(&groups);
	if (!mpc_groups_collect(&groups, &argc, argv))
		return -1;

	mpd_search_db_tags(conn, type);

	if (argc > 0 && !add_constraints(argc, argv, conn))
		return -1;

	if (!mpc_groups_send(conn, &groups))
		printErrorAndExit(conn);

	if (!mpd_search_commit(conn))
		printErrorAndExit(conn);

	if (groups.n_groups > 0) {
		struct mpd_pair *pair;
		while ((pair = mpd_recv_pair(conn)) != NULL) {
			enum mpd_tag_type t = mpd_tag_name_iparse(pair->name);
			//printf("|| %s\n", pair->name);
			if (t != MPD_TAG_UNKNOWN) {
				int i = mpc_groups_find(&groups, t);
				if (i < 0)
					i = groups.n_groups;
				if (i >= 0)
					printf("%*s%s\n", i * 4, "",
					       charset_from_utf8(pair->value));
			}
			mpd_return_pair(conn, pair);
		}
	} else {
		struct mpd_pair *pair;
		while ((pair = mpd_recv_pair_tag(conn, type)) != NULL) {
			printf("%s\n", charset_from_utf8(pair->value));
			mpd_return_pair(conn, pair);
		}
	}

	my_finishCommand(conn);
	return 0;
}

int
cmd_volume(int argc, char **argv, struct mpd_connection *conn)
{
	struct int_value_change ch;

	if (argc == 1) {
		if (!parse_int_value_change(argv[0], &ch))
			DIE("\"%s\" is not an integer\n", argv[0]);
	} else {
		struct mpd_status *status = getStatus(conn);

		if (mpd_status_get_volume(status) >= 0)
			printf("volume:%3i%c\n",
			       mpd_status_get_volume(status), '%');
		else
			printf("volume: n/a\n");

		mpd_status_free(status);
		return 0;
	}

	if (ch.is_relative) {
		struct mpd_status *status = getStatus(conn);
		int old_volume = mpd_status_get_volume(status);
		mpd_status_free(status);

		ch.value += old_volume;
		if (ch.value < 0)
			ch.value = 0;
		else if (ch.value > 100)
			ch.value = 100;

		if (ch.value == old_volume)
			return 1;
	}

	if (!mpd_run_set_volume(conn, ch.value))
		printErrorAndExit(conn);
	return 1;
}

int
cmd_pause(gcc_unused int argc, gcc_unused char **argv,
	  struct mpd_connection *conn)
{
	mpd_send_pause(conn, true);
	my_finishCommand(conn);

	return 1;
}

int
cmd_pause_if_playing(gcc_unused int argc, gcc_unused char **argv,
		     struct mpd_connection *conn)
{
	struct mpd_status *status = getStatus(conn);
	int ret = 1;

	if (mpd_status_get_state(status) != MPD_STATE_PLAY) {
		ret = -127;
	} else {
		cmd_pause(0, NULL, conn);
	}

	mpd_status_free(status);

	return ret;
}

static int
bool_cmd(int argc, char **argv, struct mpd_connection *conn,
	 bool (*get_mode)(const struct mpd_status *status),
	 bool (*run_set_mode)(struct mpd_connection *conn, bool mode))
{
	bool mode;

	if (argc == 1) {
		int mode_i = get_boolean(argv[0]);
		if (mode_i < 0)
			return -1;

		mode = (bool)mode_i;
	} else {
		struct mpd_status *status;
		status = getStatus(conn);
		mode = !get_mode(status);
		mpd_status_free(status);
	}

	if (!run_set_mode(conn, mode))
		printErrorAndExit(conn);

	return 1;
}

int
cmd_repeat(int argc, char **argv, struct mpd_connection *conn)
{
	return bool_cmd(argc, argv, conn,
			mpd_status_get_repeat, mpd_run_repeat);
}

int
cmd_random(int argc, char **argv, struct mpd_connection *conn)
{
	return bool_cmd(argc, argv, conn,
			mpd_status_get_random, mpd_run_random);
}

int
cmd_single(int argc, char **argv, struct mpd_connection *conn)
{
	enum mpd_single_state mode = MPD_SINGLE_UNKNOWN;

	if (argc == 1) {
		if (strcasecmp(argv[0], "once") == 0)
			mode = MPD_SINGLE_ONESHOT;
		else {
			int mode_i = get_boolean(argv[0]);
			if (mode_i < 0)
				return -1;
			else if (mode_i)
				mode = MPD_SINGLE_ON;
			else
				mode = MPD_SINGLE_OFF;
		}
	} else {
		struct mpd_status *status;
		status = getStatus(conn);
		enum mpd_single_state cur = mpd_status_get_single_state(status);

		if (cur == MPD_SINGLE_ONESHOT || cur == MPD_SINGLE_ON)
			mode = MPD_SINGLE_OFF;
		else if (cur == MPD_SINGLE_OFF)
			mode = MPD_SINGLE_ON;

		mpd_status_free(status);
	}

	if (mode == MPD_SINGLE_UNKNOWN)
		return -1;
	else if (!mpd_run_single_state(conn, mode))
		printErrorAndExit(conn);

	return 1;
}

int
cmd_consume(int argc, char **argv, struct mpd_connection *conn)
{
#if LIBMPDCLIENT_CHECK_VERSION(2,21,0)
	enum mpd_consume_state mode = MPD_CONSUME_UNKNOWN;

	if (argc == 1) {
		if (strcasecmp(argv[0], "once") == 0)
			mode = MPD_CONSUME_ONESHOT;
		else {
			int mode_i = get_boolean(argv[0]);
			if (mode_i < 0)
				return -1;
			else if (mode_i)
				mode = MPD_CONSUME_ON;
			else
				mode = MPD_CONSUME_OFF;
		}
	} else {
		struct mpd_status *status;
		status = getStatus(conn);
		enum mpd_consume_state cur = mpd_status_get_consume_state(status);

		if (cur == MPD_CONSUME_ONESHOT || cur == MPD_CONSUME_ON)
			mode = MPD_CONSUME_OFF;
		else if (cur == MPD_CONSUME_OFF)
			mode = MPD_CONSUME_ON;

		mpd_status_free(status);
	}

	if (mode == MPD_CONSUME_UNKNOWN)
		return -1;
	else if (!mpd_run_consume_state(conn, mode))
		printErrorAndExit(conn);

	return 1;
#else
	return bool_cmd(argc, argv, conn,
			mpd_status_get_consume, mpd_run_consume);
#endif
}

int
cmd_crossfade(int argc, char **argv, struct mpd_connection *conn)
{
	if (argc==1) {
		int seconds;
		if (!parse_int(argv[0], &seconds) || seconds < 0)
			DIE("\"%s\" is not 0 or positive integer\n", argv[0]);

		if (!mpd_run_crossfade(conn, seconds))
			printErrorAndExit(conn);
	} else {
		struct mpd_status *status;
		status = getStatus(conn);

		printf("crossfade: %i\n", mpd_status_get_crossfade(status));

		mpd_status_free(status);
	}

	return 0;
}

int
cmd_mixrampdb(int argc, char **argv, struct mpd_connection *conn)
{
	if (argc == 1) {
		float db;
		if (!parse_float(argv[0], &db))
			DIE("\"%s\" is not a floating point number\n",
			    argv[0]);

		mpd_run_mixrampdb(conn, db);
		my_finishCommand(conn);
	} else {
		struct mpd_status *status = getStatus(conn);

		printf("mixrampdb: %f\n",
		       (double)mpd_status_get_mixrampdb(status));

		mpd_status_free(status);
	}

	return 0;
}

int
cmd_mixrampdelay(int argc, char **argv, struct mpd_connection *conn)
{
	if (argc == 1) {
		float seconds;
		if (!parse_float(argv[0], &seconds))
			DIE("\"%s\" is not a floating point number\n",
			    argv[0]);

		mpd_run_mixrampdelay(conn, seconds);
		my_finishCommand(conn);
	} else {
		struct mpd_status *status = getStatus(conn);

		printf("mixrampdelay: %f\n",
		       (double)mpd_status_get_mixrampdelay(status));

		mpd_status_free(status);
	}

	return 0;
}

int
cmd_version(gcc_unused int argc, gcc_unused char **argv,
	    struct mpd_connection *conn)
{
	const unsigned *version = mpd_connection_get_server_version(conn);

	if (version != NULL)
		printf("mpd version: %i.%i.%i\n", version[0],
		       version[1], version[2]);
	else
		printf("mpd version: unknown\n");

	return 0;
}

static char *
DHMS(unsigned long t)
{
	static char buf[32];	/* Ugh */

#ifndef SECSPERDAY
#define SECSPERDAY 86400
#endif
#ifndef SECSPERHOUR
#define SECSPERHOUR 3600
#endif
#ifndef SECSPERMIN
#define SECSPERMIN 60
#endif

	unsigned days = t / SECSPERDAY;
	t %= SECSPERDAY;
	unsigned hours = t / SECSPERHOUR;
	t %= SECSPERHOUR;
	unsigned mins = t / SECSPERMIN;
	t %= SECSPERMIN;
	unsigned secs = t;

	snprintf(buf, sizeof(buf), "%d days, %d:%02d:%02d",
	    days, hours, mins, secs);
	return buf;
}

int
cmd_stats(gcc_unused int argc, gcc_unused char **argv,
	  struct mpd_connection *conn)
{
	struct mpd_stats *stats = mpd_run_stats(conn);
	if (stats == NULL)
		printErrorAndExit(conn);

	printf("Artists: %6d\n", mpd_stats_get_number_of_artists(stats));
	printf("Albums:  %6d\n", mpd_stats_get_number_of_albums(stats));
	printf("Songs:   %6d\n", mpd_stats_get_number_of_songs(stats));
	printf("\n");
	printf("Play Time:    %s\n", DHMS(mpd_stats_get_play_time(stats)));
	printf("Uptime:       %s\n", DHMS(mpd_stats_get_uptime(stats)));

	time_t t = mpd_stats_get_db_update_time(stats);
	printf("DB Updated:   %s", ctime(&t));	/* no \n needed */

	printf("DB Play Time: %s\n", DHMS(mpd_stats_get_db_play_time(stats)));

	mpd_stats_free(stats);
	return 0;
}

int
cmd_status(int argc, char **argv, struct mpd_connection *conn)
{
	if (options.verbosity >= V_DEFAULT) {
		if (argc == 0) {
			print_status(conn);
		} else if (argc == 1) {
			struct mpd_status *status = getStatus(conn);
			char* current_status = format_status(status, argv[0]);
			if (current_status) {
				printf("%s\n", format_status(status, argv[0]));
			}
		}
	}
	return 0;
}

int
cmd_replaygain(int argc, char **argv, struct mpd_connection *connection)
{
	/* libmpdclient 2.0 doesn't support these commands yet, we
	   have to roll our own with mpd_send_command() */

	if (argc == 0) {
		mpd_send_command(connection, "replay_gain_status", NULL);

		struct mpd_pair *pair;
		while ((pair = mpd_recv_pair(connection)) != NULL) {
			printf("%s: %s\n", pair->name, pair->value);
			mpd_return_pair(connection, pair);
		}

		my_finishCommand(connection);
	} else {
		mpd_send_command(connection, "replay_gain_mode",
				 argv[0], NULL);
		my_finishCommand(connection);
	}

	return 0;
}

int
cmd_partitionlist(gcc_unused int argc, gcc_unused char **argv, struct mpd_connection *conn)
{
	if (!mpd_send_listpartitions(conn)) {
		printErrorAndExit(conn);
	}

	struct mpd_partition *part;
	while ((part = mpd_recv_partition(conn)) != NULL) {
		printf("%s\n", mpd_partition_get_name(part));
		mpd_partition_free(part);
	}

	my_finishCommand(conn);
	return 0;
}

int cmd_partitionmake(int argc, char **argv, struct mpd_connection *conn)
{
	if (!mpd_command_list_begin(conn, false)) {
		printErrorAndExit(conn);
	}

	for (int i = 0; i < argc; ++i) {
		if (!mpd_send_newpartition(conn, argv[i])) {
			printErrorAndExit(conn);
		}
	}

	if (!mpd_command_list_end(conn) || !mpd_response_finish(conn)) {
		printErrorAndExit(conn);
	}
	return 0;
}

int
cmd_partitiondelete(int argc, char **argv, struct mpd_connection *conn) {
	if (!mpd_command_list_begin(conn, false)) {
		printErrorAndExit(conn);
	}

	for (int i = 0; i < argc; ++i) {
		if (!mpd_send_delete_partition(conn, argv[i])) {
			printErrorAndExit(conn);
		}
	}

	if (!mpd_command_list_end(conn) || !mpd_response_finish(conn)) {
		printErrorAndExit(conn);
	}
	return 0;
}
