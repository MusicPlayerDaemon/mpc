/* music player command (mpc)
 * Copyright (C) 2003-2008 Warren Dukes <warren.dukes@gmail.com>,
				Eric Wong <normalperson@yhbt.net>,
				Daniel Brown <danb@cs.utexas.edu>
 * Copyright (C) 2008-2009 Max Kellermann <max@duempel.org>
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

#include "list.h"
#include "charset.h"
#include "password.h"
#include "options.h"
#include "util.h"
#include "status.h"
#include "command.h"
#include "mpc.h"
#include "gcc.h"

#include <mpd/client.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/param.h>

#define DIE(...) do { fprintf(stderr, __VA_ARGS__); return -1; } while(0)

#define SIMPLE_CMD(funcname, libmpdclient_funcname, ret) \
int funcname(mpd_unused int argc, mpd_unused char **argv, \
	     struct mpd_connection *conn) { \
        libmpdclient_funcname(conn); \
        my_finishCommand(conn); \
        return ret; \
}

#define SIMPLE_ONEARG_CMD(funcname, libmpdclient_funcname, ret) \
int funcname (mpd_unused int argc, char **argv, struct mpd_connection *conn) { \
	if (!libmpdclient_funcname(conn, charset_to_utf8(argv[0]))) \
		printErrorAndExit(conn); \
        return ret; \
}

static void my_finishCommand(struct mpd_connection *conn) {
	if (!mpd_response_finish(conn))
		printErrorAndExit(conn);
}

SIMPLE_CMD(cmd_next, mpd_run_next, 1)
SIMPLE_CMD(cmd_prev, mpd_run_previous, 1)
SIMPLE_CMD(cmd_stop, mpd_run_stop, 1)
SIMPLE_CMD(cmd_clear, mpd_run_clear, 1)
SIMPLE_CMD(cmd_shuffle, mpd_run_shuffle, 1)

SIMPLE_ONEARG_CMD(cmd_save, mpd_run_save, 0)
SIMPLE_ONEARG_CMD(cmd_rm, mpd_run_rm, 0)

static struct mpd_status *
getStatus(struct mpd_connection *conn) {
	struct mpd_status *ret = mpd_run_status(conn);
	if (ret == NULL)
		printErrorAndExit(conn);

	return ret;
}

int cmd_add (int argc, char ** argv, struct mpd_connection *conn )
{
	int i;

	if (!mpd_command_list_begin(conn, false))
		printErrorAndExit(conn);

	for(i=0;i<argc;i++) {
		if (options.verbosity >= V_VERBOSE)
			printf("adding: %s\n", argv[i]);
		mpd_send_add(conn, charset_to_utf8(argv[i]));
	}

	if (!mpd_command_list_end(conn) || !mpd_response_finish(conn))
		printErrorAndExit(conn);

	return 0;
}

int
cmd_crop(mpd_unused int argc, mpd_unused char **argv, struct mpd_connection *conn)
{
	struct mpd_status *status = getStatus( conn );
	int length = mpd_status_get_queue_length(status) - 1;

	if (length < 0) {

		mpd_status_free(status);
		DIE( "You have to have a playlist longer than 1 song in length to crop" );

	} else if (mpd_status_get_state(status) == MPD_STATE_PLAY ||
		   mpd_status_get_state(status) == MPD_STATE_PAUSE) {
		if (!mpd_command_list_begin(conn, false))
			printErrorAndExit(conn);

		while( length >= 0 )
		{
			if (length != mpd_status_get_song_pos(status)) {
				mpd_send_delete(conn, length);
			}
			length--;
		}

		mpd_status_free(status);

		if (!mpd_command_list_end(conn) || !mpd_response_finish(conn))
			printErrorAndExit(conn);

		return ( 0 );

	} else {

		mpd_status_free(status);
		DIE( "You need to be playing to crop the playlist\n" );

	}
}

int cmd_current(mpd_unused int argc, mpd_unused char ** argv, struct mpd_connection *conn)
{
	struct mpd_status *status;

	if (!mpd_command_list_begin(conn, true) ||
	    !mpd_send_status(conn) ||
	    !mpd_send_current_song(conn) ||
	    !mpd_command_list_end(conn))
		printErrorAndExit(conn);

	status = mpd_recv_status(conn);
	if (status == NULL)
		printErrorAndExit(conn);

	if (mpd_status_get_state(status) == MPD_STATE_PLAY ||
	    mpd_status_get_state(status) == MPD_STATE_PAUSE) {
		struct mpd_song *song;

		if (!mpd_response_next(conn))
			printErrorAndExit(conn);

		song = mpd_recv_song(conn);
		if (song != NULL) {
			pretty_print_song(song);
			printf("\n");

			mpd_song_free(song);
		}

		if (!mpd_response_finish(conn))
			printErrorAndExit(conn);
	}
	mpd_status_free(status);

	return 0;
}

int cmd_del ( int argc, char ** argv, struct mpd_connection *conn )
{
	int i,j;
	char * s;
	char * t;
	char * t2;
	int range[2];
	int songsDeleted = 0;
	int plLength = 0;
	char * songsToDel;
	struct mpd_status *status;

	status = getStatus(conn);

	plLength = mpd_status_get_queue_length(status);

	songsToDel = malloc(plLength);
	memset(songsToDel,0,plLength);

	for(i=0;i<argc;i++) {
		if(argv[i][0]=='#') s = &(argv[i][1]);
		else s = argv[i];

		range[0] = strtol(s,&t,10);

		/* If argument is 0 current song and we're not stopped */
		if(range[0] == 0 && strlen(s) == 1 && \
			(mpd_status_get_state(status) == MPD_STATE_PLAY ||
			 mpd_status_get_state(status) == MPD_STATE_PAUSE))
			range[0] = mpd_status_get_song_pos(status) + 1;

		if(s==t)
			DIE("error parsing song numbers from: %s\n",argv[i]);
		else if(*t=='-') {
			range[1] = strtol(t+1,&t2,10);
			if(t+1==t2 || *t2!='\0')
				DIE("error parsing range from: %s\n",argv[i]);
		}
		else if(*t==')' || *t=='\0') range[1] = range[0];
		else
			DIE("error parsing song numbers from: %s\n",argv[i]);

		if(range[0]<=0 || range[1]<=0) {
			if (range[0]==range[1])
				DIE("song number must be positive: %i\n",range[0]);
			else
				DIE("song numbers must be positive: %i to %i\n",range[0],range[1]);
		}

		if(range[1]<range[0])
			DIE("song range must be from low to high: %i to %i\n",range[0],range[1]);

		if(range[1]>plLength)
			DIE("song number does not exist: %i\n",range[1]);

		for(j=range[0];j<=range[1];j++) songsToDel[j-1] = 1;
	}

	if (!mpd_command_list_begin(conn, false))
		printErrorAndExit(conn);

	for(i=0;i<plLength;i++) {
		if(songsToDel[i]) {
			mpd_send_delete(conn, i - songsDeleted);
			songsDeleted++;
		}
	}

	mpd_status_free(status);
	free(songsToDel);

	if (!mpd_command_list_end(conn) || !mpd_response_finish(conn))
		printErrorAndExit(conn);

	return 0;
}

int
cmd_toggle(mpd_unused int argc, mpd_unused char **argv, struct mpd_connection *conn)
{
	struct mpd_status *status;
	status = getStatus(conn);

	if (mpd_status_get_state(status) == MPD_STATE_PLAY) {
		cmd_pause(0, NULL, conn);
	} else {
		cmd_play(0, NULL, conn);
	}
	return 1;
}

int
cmd_outputs(mpd_unused int argc, mpd_unused char **argv, struct mpd_connection *conn)
{
	struct mpd_output *output;

	mpd_send_outputs(conn);

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
	mpd_response_finish(conn);
	return( 0 );
}

int
cmd_enable(mpd_unused int argc, char **argv, struct mpd_connection *conn)
{
	int arg;

        if( ! parse_int( argv[0], &arg ) || arg <= 0 ) {
		DIE( "Not a positive integer\n" );
	}

	mpd_run_enable_output(conn, arg - 1);
	return cmd_outputs(0, NULL, conn);
}

int
cmd_disable(mpd_unused int argc, char **argv, struct mpd_connection *conn)
{
	int arg;

        if( ! parse_int( argv[0], &arg ) || arg <= 0 ) {
		DIE( "Not a positive integer\n" );
	}

	mpd_run_disable_output(conn, arg - 1);
	return cmd_outputs(0, NULL, conn);
}

int cmd_play ( int argc, char ** argv, struct mpd_connection *conn )
{
	int song;
	int i;

	if(0==argc) song = -1;
	else {
		struct mpd_status *status;

		for(i=0;i<argc-1;i++)
			printf("skipping: %s\n",argv[i]);

                if(!parse_songnum(argv[i], &song))
			DIE("error parsing song numbers from: %s\n",argv[i]);

		song--;

		/* This is necessary, otherwise mpc will output the wrong playlist number */
		status = getStatus(conn);
		i = mpd_status_get_queue_length(status);
		mpd_status_free(status);
		if(song >= i)
			DIE("song number greater than playlist length.\n");
	}

	if (song >= 0)
		mpd_run_play_pos(conn, song);
	else
		mpd_run_play(conn);

	return 1;
}

int
cmd_seek(mpd_unused int argc, mpd_unused char **argv, struct mpd_connection *conn)
{
	struct mpd_status *status;
	char * arg = argv[0];
	char * test;

	int seekchange;
	int total_secs;
	int seekto;
        int rel = 0;

	status = getStatus(conn);

	if (mpd_status_get_state(status) == MPD_STATE_STOP)
		DIE("not currently playing\n");

	/* Detect +/- if exists point to the next char */
        if(*arg == '+') rel = 1;
        else if(*arg == '-') rel = -1;

	if(rel != 0) arg++;

	/* If seeking by percent */
	if( arg[strlen(arg)-1] == '%' ) {

		double perc;

		/* Remove the % */
		arg[ strlen(arg) - 1 ] = '\0';

		/* percent seek, strtod is needed for percent with decimals */
		perc = strtod(arg,&test);

		if(( *test!='\0' ) || (!rel && (perc<0 || perc>100)) || (rel && perc>abs(100)))
			DIE("\"%s\" is not an number between 0 and 100\n",arg);

		seekchange = perc * mpd_status_get_total_time(status) / 100 + 0.5;

	} else { /* If seeking by absolute seek time */

		if( strchr( arg, ':' )) {
			char * sec_ptr;
			char * min_ptr;
			char * hr_ptr;

			int hr = 0;
			int min = 0;
			int sec = 0;

			/* Take the seconds off the end of arg */
			sec_ptr = strrchr( arg, ':' );

			/* Remove ':' and move the pointer one byte up */
			* sec_ptr = '\0';
			++sec_ptr;

			/* If hour is in the argument, else just point to the arg */
			if(( min_ptr = strrchr( arg, ':' ))) {

				/* Remove ':' and move the pointer one byte up */
				* min_ptr = '\0';
				++min_ptr;

				/* If the argument still exists, it's the hour  */
				if( arg != NULL ) {
					hr_ptr = arg;
					hr = strtol( hr_ptr, &test, 10 );

					if( *test != '\0' || ( ! rel && hr < 0 ))
						DIE("\"%s\" is not a positive number\n", sec_ptr);
				}
			} else {
				min_ptr = arg;
			}

			/* Change the pointers to a integer  */
			sec = strtol( sec_ptr, &test, 10 );

			if( *test != '\0' || ( ! rel && sec < 0 ))
				DIE("\"%s\" is not a positive number\n", sec_ptr);

			min = strtol( min_ptr, &test, 10 );

			if( *test != '\0' || ( ! rel && min < 0 ))
				DIE("\"%s\" is not a positive number\n", min_ptr);

			/* If mins exist, check secs. If hrs exist, check mins  */
			if( min && strlen(sec_ptr) != 2 )
				DIE("\"%s\" is not two digits\n", sec_ptr);
			else if( hr && strlen(min_ptr) != 2 )
				DIE("\"%s\" is not two digits\n", min_ptr);

			/* Finally, make sure they're not above 60 if higher unit exists */
			if( min && sec > 60 )
				DIE("\"%s\" is greater than 60\n", sec_ptr);
			else if( hr && min > 60 )
				DIE("\"%s\" is greater than 60\n", min_ptr);

			total_secs = ( hr * 3600 ) + ( min * 60 ) + sec;

		} else {

			/* absolute seek (in seconds) */
			total_secs = strtol( arg, &test, 10 ); /* get the # of seconds */

			if( *test != '\0' || ( ! rel && total_secs < 0 ))
				DIE("\"%s\" is not a positive number\n", arg);
		}
		seekchange = total_secs;
	}

	/* This detects +/- and is necessary due to the parsing of HH:MM:SS numbers*/
	if(rel == 1) {
		seekto = mpd_status_get_elapsed_time(status) + seekchange;
	} else if (rel == -1) {
		seekto = mpd_status_get_elapsed_time(status) - seekchange;
	} else {
		seekto = seekchange;
	}

	if (seekto > (int)mpd_status_get_total_time(status))
		DIE("Seek amount would seek past the end of the song\n");

	mpd_status_free(status);

	if (!mpd_run_seek_id(conn, mpd_status_get_song_id(status), seekto))
		printErrorAndExit(conn);

	return 1;
}

int
cmd_move(mpd_unused int argc, char **argv, struct mpd_connection *conn)
{
	int from;
	int to;

	if(!parse_int(argv[0], &from) || from<=0)
		DIE("\"%s\" is not a positive integer\n",argv[0]);

	if(!parse_int(argv[1], &to) || to<=0)
		DIE("\"%s\" is not a positive integer\n",argv[1]);

	/* users type in 1-based numbers, mpd uses 0-based */
	from--;
	to--;

	mpd_run_move(conn, from, to);

	return 0;
}

int
cmd_playlist(mpd_unused int argc, mpd_unused char **argv, struct mpd_connection *conn)
{
	struct mpd_song *song;

	if (!mpd_send_list_queue_meta(conn))
		printErrorAndExit(conn);

	while ((song = mpd_recv_song(conn)) != NULL) {
		pretty_print_song(song);
		mpd_song_free(song);
		printf("\n");
	}

	my_finishCommand(conn);

	return 0;
}

int cmd_listall ( int argc, char ** argv, struct mpd_connection *conn )
{
	const char * listall = "";
	int i=0;

	if (argc > 0)
		listall = charset_to_utf8(argv[i]);

	do {
		if (!mpd_send_list_all(conn, listall))
			printErrorAndExit(conn);

		print_filenames(conn);

		my_finishCommand(conn);

	} while (++i < argc && (listall = charset_to_utf8(argv[i])) != NULL);

	return 0;
}

int cmd_update ( int argc, char ** argv, struct mpd_connection *conn)
{
	const char * update = "";
	int i = 0;
	unsigned id = 0;

	if (!mpd_command_list_begin(conn, false))
		printErrorAndExit(conn);

	if(argc > 0) update = charset_to_utf8(argv[i]);

	do {
		mpd_send_update(conn, update);
	} while (++i < argc && (update = charset_to_utf8(argv[i])) != NULL);

	if (!mpd_command_list_end(conn))
		printErrorAndExit(conn);

	/* obtain the last "update id" response */

	while (true) {
		unsigned next_id = mpd_recv_update_id(conn);
		if (next_id == 0)
			break;
		id = next_id;
	}

	if (!mpd_response_finish(conn))
		printErrorAndExit(conn);

	while (options.wait) {
		/* idle until an update finishes */
		enum mpd_idle idle = mpd_run_idle_mask(conn, MPD_IDLE_UPDATE);
		struct mpd_status *status;
		unsigned current_id;

		if (idle == 0)
			printErrorAndExit(conn);

		/* determine the current "update id" */

		status = mpd_run_status(conn);
		if (status == NULL)
			printErrorAndExit(conn);

		current_id = mpd_status_get_update_id(status);
		mpd_status_free(status);

		/* is our last queued update finished now? */

		if (current_id == 0 || current_id > id ||
		    (id > 1 << 30 && id < 1000)) /* wraparound */
			break;
	}

	return 1;
}

int cmd_ls ( int argc, char ** argv, struct mpd_connection *conn )
{
	struct mpd_entity *entity;
	const char *ls;
	int i = 0;

	if (argc > 0)
		ls = charset_to_utf8(argv[i]);
	else
		ls = strdup("");

	do {
		if (!mpd_send_list_meta(conn, ls))
			printErrorAndExit(conn);

		while ((entity = mpd_recv_entity(conn)) != NULL) {
			const struct mpd_directory *dir;
			const struct mpd_song *song;

			switch (mpd_entity_get_type(entity)) {
			case MPD_ENTITY_TYPE_UNKNOWN:
				break;

			case MPD_ENTITY_TYPE_DIRECTORY:
				dir = mpd_entity_get_directory(entity);
				printf("%s\n", charset_from_utf8(mpd_directory_get_path(dir)));
				break;

			case MPD_ENTITY_TYPE_SONG:
				song = mpd_entity_get_song(entity);
				printf("%s\n", charset_from_utf8(mpd_song_get_uri(song)));
				break;

			case MPD_ENTITY_TYPE_PLAYLIST:
				break;
			}

			mpd_entity_free(entity);
		}

		my_finishCommand(conn);

	} while (++i < argc && (ls = charset_to_utf8(argv[i])) != NULL);

	return 0;
}

int cmd_lsplaylists ( int argc, char ** argv, struct mpd_connection *conn )
{
	struct mpd_entity *entity;
	const char * ls = "";
	int i = 0;

	if(argc>0) ls = charset_to_utf8(argv[i]);

	do {
		if (!mpd_send_list_meta(conn, ls))
			printErrorAndExit(conn);

		while ((entity = mpd_recv_entity(conn)) != NULL) {
			if (mpd_entity_get_type(entity) == MPD_ENTITY_TYPE_PLAYLIST) {
				const struct mpd_playlist *playlist =
					mpd_entity_get_playlist(entity);

				printf("%s\n", charset_from_utf8(mpd_playlist_get_path(playlist)));
			}
			mpd_entity_free(entity);
		}

		my_finishCommand(conn);

	} while (++i < argc && (ls = charset_to_utf8(argv[i])) != NULL);
	return 0;
}

int cmd_load ( int argc, char ** argv, struct mpd_connection *conn )
{
	int i;

	if (!mpd_command_list_begin(conn, false))
		printErrorAndExit(conn);

	for(i=0;i<argc;i++) {
		printf("loading: %s\n",argv[i]);
		mpd_send_load(conn, charset_to_utf8(argv[i]));
	}
	mpd_command_list_end(conn);
	my_finishCommand(conn);

	return 0;
}

static int do_search ( int argc, char ** argv, struct mpd_connection *conn, int exact )
{
	Constraint *constraints;
	int numconstraints;
	int i;

	if (argc % 2 != 0)
		DIE("arguments must be pairs of search types and queries\n");

	numconstraints = get_constraints(argc, argv, &constraints);
	if (numconstraints < 0)
		return -1;

	mpd_search_db_songs(conn, exact);

	for (i = 0; i < numconstraints; i++) {
		mpd_search_add_tag_constraint(conn, MPD_OPERATOR_DEFAULT,
					      constraints[i].type,
					      charset_to_utf8(constraints[i].query));
	}

	free(constraints);

	if (!mpd_search_commit(conn))
		printErrorAndExit(conn);

	print_filenames(conn);

	my_finishCommand(conn);

	return 0;
}

int cmd_search ( int argc, char ** argv, struct mpd_connection *conn )
{
	return do_search(argc, argv, conn, 0);
}

int cmd_find ( int argc, char ** argv, struct mpd_connection *conn )
{
	return do_search(argc, argv, conn, 1);
}

int cmd_list ( int argc, char ** argv, struct mpd_connection *conn )
{
	Constraint *constraints;
	int numconstraints = 0;
	int type;
	int i;
	struct mpd_pair *pair;

	type = get_search_type(argv[0]);
	if (type < 0)
		return -1;

	argc -= 1;
	argv += 1;

	if (argc > 0) {
		if (argc % 2 != 0) {
			DIE("arguments must be a tag type and "
			    "optional pairs of search types and queries\n");
		}

		numconstraints = get_constraints(argc, argv, &constraints);
		if (numconstraints < 0)
			return -1;
	}

	mpd_search_db_tags(conn, type);

	if (argc > 0) {
		for (i = 0; i < numconstraints; i++) {
			mpd_search_add_tag_constraint(conn,
						      MPD_OPERATOR_DEFAULT,
						      constraints[i].type,
						  charset_to_utf8(constraints[i].query));
		}

		free(constraints);
	}

	if (!mpd_search_commit(conn))
		printErrorAndExit(conn);

	while ((pair = mpd_recv_pair_tag(conn, type)) != NULL) {
		printf("%s\n", charset_from_utf8(pair->value));
		mpd_return_pair(conn, pair);
	}

	my_finishCommand(conn);

	return 0;
}

int cmd_volume ( int argc, char ** argv, struct mpd_connection *conn )
{
        struct int_value_change ch;
	struct mpd_status *status;

	if(argc==1) {
                if(!parse_int_value_change(argv[0], &ch))
			DIE("\"%s\" is not an integer\n", argv[0]);
	} else {
		status = getStatus(conn);

		printf("volume:%3u%c   \n",
		       mpd_status_get_volume(status), '%');

		mpd_status_free(status);

		return 0;
	}

	if (ch.is_relative) {
		int old_volume;

		status = getStatus(conn);
		old_volume = mpd_status_get_volume(status);
		mpd_status_free(status);

		ch.value += old_volume;
		if (ch.value < 0)
			ch.value = 0;
		else if (ch.value > 100)
			ch.value = 100;

		if (ch.value == old_volume)
			return 0;
	}

	if (!mpd_run_set_volume(conn, ch.value))
		printErrorAndExit(conn);
	return 1;
}

int
cmd_pause(mpd_unused int argc, mpd_unused char **argv, struct mpd_connection *conn)
{
	mpd_send_pause(conn, true);
	my_finishCommand(conn);

	return 1;
}

int cmd_repeat ( int argc, char ** argv, struct mpd_connection *conn )
{
	bool mode;

	if(argc==1) {
		mode = get_boolean(argv[0]);
		if (mode < 0)
			return -1;
	}
	else {
		struct mpd_status *status;
		status = getStatus(conn);
		mode = !mpd_status_get_repeat(status);
		mpd_status_free(status);
	}

	if (!mpd_run_repeat(conn, mode))
		printErrorAndExit(conn);

	return 1;
}

int cmd_random ( int argc, char ** argv, struct mpd_connection *conn )
{
	bool mode;

	if(argc==1) {
		mode = get_boolean(argv[0]);
		if (mode < 0)
			return -1;
	}
	else {
		struct mpd_status *status;
		status = getStatus(conn);
		mode = !mpd_status_get_random(status);
		mpd_status_free(status);
	}

	if (!mpd_run_random(conn, mode))
		printErrorAndExit(conn);

	return 1;
}

int cmd_single ( int argc, char ** argv, struct mpd_connection *conn )
{
	bool mode;

	if(argc==1) {
		mode = get_boolean(argv[0]);
		if (mode < 0)
			return -1;
	}
	else {
		struct mpd_status *status;
		status = getStatus(conn);
		mode = !mpd_status_get_single(status);
		mpd_status_free(status);
	}

	if (!mpd_run_single(conn, mode))
		printErrorAndExit(conn);

	return 1;
}

int cmd_consume ( int argc, char ** argv, struct mpd_connection *conn )
{
	int mode;

	if(argc==1) {
		mode = get_boolean(argv[0]);
		if (mode < 0)
			return -1;
	}
	else {
		struct mpd_status *status;
		status = getStatus(conn);
		mode = !mpd_status_get_consume(status);
		mpd_status_free(status);
	}

	if (!mpd_run_consume(conn, mode))
		printErrorAndExit(conn);

	return 1;
}

int cmd_crossfade ( int argc, char ** argv, struct mpd_connection *conn )
{
	int seconds;

	if(argc==1) {
                if(!parse_int(argv[0], &seconds) || seconds<0)
			DIE("\"%s\" is not 0 or positive integer\n",argv[0]);

		if (!mpd_run_crossfade(conn, seconds))
			printErrorAndExit(conn);
	}
	else {
		struct mpd_status *status;
		status = getStatus(conn);

		printf("crossfade: %i\n", mpd_status_get_crossfade(status));

		mpd_status_free(status);
	}
	return 0;
}

int
cmd_version(mpd_unused int argc, mpd_unused char **argv, struct mpd_connection *conn)
{
	const unsigned *version = mpd_connection_get_server_version(conn);

	if (version != NULL)
		printf("mpd version: %i.%i.%i\n", version[0],
		       version[1], version[2]);
	else
		printf("mpd version: unknown\n");

	return 0;
}

int cmd_loadtab ( int argc, char ** argv, struct mpd_connection *conn )
{
	struct mpd_playlist *pl;

	if (argc != 1)
		return 0;

	if (!mpd_send_list_meta(conn, NULL))
		printErrorAndExit(conn);

	while ((pl = mpd_recv_playlist(conn)) != NULL) {
		if (strncmp(mpd_playlist_get_path(pl), argv[0],
			    strlen(argv[0])) == 0)
			printf("%s\n",
			       charset_from_utf8(mpd_playlist_get_path(pl)));

		mpd_playlist_free(pl);
	}

	my_finishCommand(conn);
	return 0;
}

int cmd_lstab ( int argc, char ** argv, struct mpd_connection *conn )
{
	struct mpd_directory *dir;

	if (argc != 1)
		return 0;

	if (!mpd_send_list_all(conn, NULL))
		printErrorAndExit(conn);

	while ((dir = mpd_recv_directory(conn)) != NULL) {
		if (strncmp(mpd_directory_get_path(dir), argv[0],
			    strlen(argv[0])) == 0)
			printf("%s\n",
			       charset_from_utf8(mpd_directory_get_path(dir)));

		mpd_directory_free(dir);
	}

	my_finishCommand(conn);

	return 0;
}

int cmd_tab ( int argc, char ** argv, struct mpd_connection *conn )
{
	struct mpd_song *song;

	if (!mpd_send_list_all(conn, ""))
		printErrorAndExit(conn);

	while ((song = mpd_recv_song(conn)) != NULL) {
		if (argc != 1 ||
		    strncmp(mpd_song_get_uri(song), argv[0],
			    strlen(argv[0])) == 0)
			printf("%s\n",
			       charset_from_utf8(mpd_song_get_uri(song)));

		mpd_song_free(song);
	}

	my_finishCommand(conn);
	return 0;
}

static char * DHMS(unsigned long t)
{
	static char buf[32];	/* Ugh */
	int days, hours, mins, secs;

#ifndef SECSPERDAY
#define SECSPERDAY 86400
#endif
#ifndef SECSPERHOUR
#define SECSPERHOUR 3600
#endif
#ifndef SECSPERMIN
#define SECSPERMIN 60
#endif

	days = t / SECSPERDAY;
	t %= SECSPERDAY;
	hours = t / SECSPERHOUR;
	t %= SECSPERHOUR;
	mins = t / SECSPERMIN;
	t %= SECSPERMIN;
	secs = t;

	snprintf(buf, sizeof(buf) - 1, "%d days, %d:%02d:%02d",
	    days, hours, mins, secs);
	return buf;
}

int
cmd_stats(mpd_unused int argc, mpd_unused char **argv, struct mpd_connection *conn)
{
	struct mpd_stats *stats;
	time_t t;

	stats = mpd_run_stats(conn);
	if (stats == NULL)
		printErrorAndExit(conn);

	t = mpd_stats_get_db_update_time(stats);
	printf("Artists: %6d\n", mpd_stats_get_number_of_artists(stats));
	printf("Albums:  %6d\n", mpd_stats_get_number_of_albums(stats));
	printf("Songs:   %6d\n", mpd_stats_get_number_of_songs(stats));
	printf("\n");
	printf("Play Time:    %s\n", DHMS(mpd_stats_get_play_time(stats)));
	printf("Uptime:       %s\n", DHMS(mpd_stats_get_uptime(stats)));
	printf("DB Updated:   %s", ctime(&t));	/* no \n needed */
	printf("DB Play Time: %s\n", DHMS(mpd_stats_get_db_play_time(stats)));

	mpd_stats_free(stats);
	return 0;
}

int
cmd_status(mpd_unused  int argc, mpd_unused char **argv, struct mpd_connection *conn)
{
	if (options.verbosity >= V_DEFAULT)
		print_status(conn);
	return 0;
}
