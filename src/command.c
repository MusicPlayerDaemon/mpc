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

#include "libmpdclient.h"
#include "list.h"
#include "charset.h"
#include "password.h"
#include "util.h"
#include "status.h"
#include "command.h"
#include "mpc.h"
#include "gcc.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/param.h>

#define DIE(...) do { fprintf(stderr, __VA_ARGS__); return -1; } while(0)

#define SIMPLE_CMD(funcname, libmpdclient_funcname, ret) \
int funcname(mpd_unused int argc, mpd_unused char **argv, \
	     mpd_Connection *conn) { \
        libmpdclient_funcname(conn); \
        my_finishCommand(conn); \
        return ret; \
}

#define SIMPLE_ONEARG_CMD(funcname, libmpdclient_funcname, ret) \
int funcname (mpd_unused int argc, char **argv, mpd_Connection *conn) { \
        libmpdclient_funcname(conn, charset_to_utf8(argv[0])); \
        my_finishCommand(conn); \
        return ret; \
}

static void my_finishCommand(mpd_Connection * conn) {
	printErrorAndExit(conn);
	mpd_finishCommand(conn);
	printErrorAndExit(conn);
}

SIMPLE_CMD(cmd_next, mpd_sendNextCommand, 1)
SIMPLE_CMD(cmd_prev, mpd_sendPrevCommand, 1)
SIMPLE_CMD(cmd_stop, mpd_sendStopCommand, 1)
SIMPLE_CMD(cmd_clear, mpd_sendClearCommand, 1)
SIMPLE_CMD(cmd_shuffle, mpd_sendShuffleCommand, 1)

SIMPLE_ONEARG_CMD(cmd_save, mpd_sendSaveCommand, 0)
SIMPLE_ONEARG_CMD(cmd_rm, mpd_sendRmCommand, 0)

static mpd_Status * getStatus(mpd_Connection * conn) {
	mpd_Status * ret;

	mpd_sendStatusCommand(conn);
	printErrorAndExit(conn);

	ret = mpd_getStatus(conn);
	printErrorAndExit(conn);

	mpd_finishCommand(conn);
	printErrorAndExit(conn);

	return ret;
}

int cmd_add (int argc, char ** argv, mpd_Connection * conn ) 
{
	int i;

	mpd_sendCommandListBegin(conn);
	printErrorAndExit(conn);

	for(i=0;i<argc;i++) {
		printf("adding: %s\n", argv[i]);
		mpd_sendAddCommand(conn, charset_to_utf8(argv[i]));
		printErrorAndExit(conn);
	}

	mpd_sendCommandListEnd(conn);
	my_finishCommand(conn);

	return 0;
}

int
cmd_crop(mpd_unused int argc, mpd_unused char **argv, mpd_Connection *conn)
{
	mpd_Status *status = getStatus( conn );
	int length = ( status->playlistLength - 1 );

	if( status->playlistLength == 0 ) {

		mpd_freeStatus(status);
		DIE( "You have to have a playlist longer than 1 song in length to crop" );

	} else if( status->state == 3 || status->state == 2 ) { /* If playing or paused */

		mpd_sendCommandListBegin( conn );
		printErrorAndExit( conn );

		while( length >= 0 )
		{
			if( length != status->song )
			{
				mpd_sendDeleteCommand( conn, length );
				printErrorAndExit( conn );
			}
			length--;
		}

		mpd_sendCommandListEnd( conn );
		my_finishCommand( conn );
		mpd_freeStatus( status );
		return ( 0 );

	} else {

		mpd_freeStatus(status);	
		DIE( "You need to be playing to crop the playlist\n" );

	}
}

int cmd_del ( int argc, char ** argv, mpd_Connection * conn )
{
	int i,j;
	char * s;
	char * t;
	char * t2;
	int range[2];
	int songsDeleted = 0;
	int plLength = 0;
	char * songsToDel;
	mpd_Status * status;

	status = getStatus(conn);

	plLength = status->playlistLength;

	songsToDel = malloc(plLength);
	memset(songsToDel,0,plLength);

	for(i=0;i<argc;i++) {
		if(argv[i][0]=='#') s = &(argv[i][1]);
		else s = argv[i];

		range[0] = strtol(s,&t,10);

		/* If argument is 0 current song and we're not stopped */
		if(range[0] == 0 && strlen(s) == 1 && \
			(status->state == MPD_STATUS_STATE_PLAY ||
			status->state == MPD_STATUS_STATE_PAUSE))
			range[0] = status->song+1;

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

	mpd_sendCommandListBegin(conn);
	printErrorAndExit(conn);
	for(i=0;i<plLength;i++) {
		if(songsToDel[i]) {
			mpd_sendDeleteCommand(conn,i-songsDeleted);
			printErrorAndExit(conn);
			songsDeleted++;
		}
	}
	mpd_sendCommandListEnd(conn);
	my_finishCommand(conn);

	mpd_freeStatus(status);
	free(songsToDel);
	return 0;
}

int
cmd_toggle(mpd_unused int argc, mpd_unused char **argv, mpd_Connection *conn)
{
	mpd_Status * status;
	status = getStatus(conn);

	if(status->state==MPD_STATUS_STATE_PLAY) {
		cmd_pause(0, NULL, conn);
	} else {
		cmd_play(0, NULL, conn);
	}
	return 1;
}

int
cmd_outputs(mpd_unused int argc, mpd_unused char **argv, mpd_Connection *conn)
{
	mpd_OutputEntity * output;

	mpd_sendOutputsCommand( conn );
	while(( output = mpd_getNextOutput( conn ))) {
		/* We increment by 1 to make it natural to the user  */
		output->id++;

		/* If it's a negative number a password is needed  */
		if( output->id > 0 )
		{
			if( output->enabled ) {
				printf( "Output %i (%s) is enabled\n", output->id, output->name );
			} else {
				printf( "Output %i (%s) is disabled\n", output->id, output->name );
			}
		}
		else
		{
			DIE( "cannot receive the current outputs\n" );
		}
		mpd_freeOutputElement( output );
	}
	mpd_finishCommand( conn );
	return( 0 );
}

int
cmd_enable(mpd_unused int argc, char **argv, mpd_Connection *conn)
{
	int arg;

        if( ! parse_int( argv[0], &arg ) || arg <= 0 ) {
		DIE( "Not a positive integer\n" );
	} else {
		mpd_sendEnableOutputCommand( conn, (arg - 1) );
	}

	mpd_finishCommand( conn );
	return cmd_outputs(0, NULL, conn);
}

int
cmd_disable(mpd_unused int argc, char **argv, mpd_Connection *conn)
{
	int arg;

        if( ! parse_int( argv[0], &arg ) || arg <= 0 ) {
		DIE( "Not a positive integer\n" );
	} else {
		mpd_sendDisableOutputCommand( conn, ( arg - 1 ) );
	}

	mpd_finishCommand( conn );
	return cmd_outputs(0, NULL, conn);
}

int cmd_play ( int argc, char ** argv, mpd_Connection * conn )
{
	int song;
	int i;

	if(0==argc) song = MPD_PLAY_AT_BEGINNING;
	else {
		mpd_Status *status;

		for(i=0;i<argc-1;i++)
			printf("skipping: %s\n",argv[i]);

                if(!parse_songnum(argv[i], &song))
			DIE("error parsing song numbers from: %s\n",argv[i]);

		song--;

		/* This is necessary, otherwise mpc will output the wrong playlist number */
		status = getStatus(conn);
		i = status->playlistLength;
		mpd_freeStatus(status);
		if(song >= i)
			DIE("song number greater than playlist length.\n");
	}

	mpd_sendPlayCommand(conn,song);
	my_finishCommand(conn);

	return 1;
}

int
cmd_seek(mpd_unused int argc, mpd_unused char **argv, mpd_Connection *conn)
{
	mpd_Status * status;
	char * arg = argv[0];
	char * test;

	int seekchange;
	int total_secs;
	int seekto;
        int rel = 0;

	status = getStatus(conn);

	if(status->state==MPD_STATUS_STATE_STOP)
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

		seekchange = perc*status->totalTime/100+0.5;

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
		seekto = status->elapsedTime + seekchange;
	} else if (rel == -1) {
		seekto = status->elapsedTime - seekchange;
	} else {
		seekto = seekchange;
	}

	if(seekto > status->totalTime)
		DIE("Seek amount would seek past the end of the song\n");

	mpd_sendSeekIdCommand(conn,status->songid,seekto);
	printErrorAndExit(conn);
	my_finishCommand(conn);
	printErrorAndExit(conn);
	mpd_freeStatus(status);
	printErrorAndExit(conn);
	return 1;
}

int
cmd_move(mpd_unused int argc, char **argv, mpd_Connection *conn)
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

	mpd_sendMoveCommand(conn,from,to);
	my_finishCommand(conn);

	return 0;
}

int
cmd_playlist(mpd_unused int argc, mpd_unused char **argv, mpd_Connection *conn)
{
	mpd_InfoEntity * entity;
	mpd_Status * status;
	int count = 0;

	mpd_sendStatusCommand(conn);
	printErrorAndExit(conn);
	status = mpd_getStatus(conn);
	printErrorAndExit(conn);
	mpd_finishCommand(conn);
	mpd_sendPlaylistInfoCommand(conn,-1);
	printErrorAndExit(conn);

	while((entity = mpd_getNextInfoEntity(conn))) {
		if(entity->type==MPD_INFO_ENTITY_TYPE_SONG) {
			struct mpd_song *song = entity->info.song;

			printf("%s%i) ", (status->song == count)?">":" ", 1+count);
			pretty_print_song(song);
			printf("\n");

			count++;
		}
		mpd_freeInfoEntity(entity);
	}

	my_finishCommand(conn);
	mpd_freeStatus(status);

	return 0;
}

int cmd_listall ( int argc, char ** argv, mpd_Connection * conn )
{
	const char * listall = "";
	int i=0;

	if (argc > 0)
		listall = charset_to_utf8(argv[i]);

	do {
		mpd_sendListallCommand(conn,listall);
		printErrorAndExit(conn);

		print_filenames(conn);

		my_finishCommand(conn);

	} while (++i < argc && (listall = charset_to_utf8(argv[i])) != NULL);

	return 0;
}

int cmd_update ( int argc, char ** argv, mpd_Connection * conn) 
{
	const char * update = "";
	int i = 0;

	mpd_sendCommandListBegin(conn);
	printErrorAndExit(conn);

	if(argc > 0) update = charset_to_utf8(argv[i]);

	do {
		mpd_sendUpdateCommand(conn, update);
	} while (++i < argc && (update = charset_to_utf8(argv[i])) != NULL);

	mpd_sendCommandListEnd(conn);
	printErrorAndExit(conn);
	mpd_finishCommand(conn);
	printErrorAndExit(conn);

	return 1;
}

int cmd_ls ( int argc, char ** argv, mpd_Connection * conn )
{
	mpd_InfoEntity * entity;
	const char *ls;
	int i = 0;

	if (argc > 0)
		ls = charset_to_utf8(argv[i]);
	else
		ls = strdup("");

	do {
		mpd_sendLsInfoCommand(conn,ls);
		printErrorAndExit(conn);

		while((entity = mpd_getNextInfoEntity(conn))) {
			if(entity->type==MPD_INFO_ENTITY_TYPE_DIRECTORY) {
				mpd_Directory * dir = entity->info.directory;
				printf("%s\n", charset_from_utf8(dir->path));
			}
			else if(entity->type==MPD_INFO_ENTITY_TYPE_SONG) {
				struct mpd_song *song = entity->info.song;
				printf("%s\n", charset_from_utf8(song->file));
			}
			mpd_freeInfoEntity(entity);
		}

		my_finishCommand(conn);

	} while (++i < argc && (ls = charset_to_utf8(argv[i])) != NULL);

	return 0;
}

int cmd_lsplaylists ( int argc, char ** argv, mpd_Connection * conn )
{
	mpd_InfoEntity * entity;
	const char * ls = "";
	int i = 0;

	if(argc>0) ls = charset_to_utf8(argv[i]);

	do {
		mpd_sendLsInfoCommand(conn,ls);
		printErrorAndExit(conn);

		while((entity = mpd_getNextInfoEntity(conn))) {
			if(entity->type==
					MPD_INFO_ENTITY_TYPE_PLAYLISTFILE) {
				mpd_PlaylistFile * pl = entity->info.playlistFile;
				printf("%s\n", charset_from_utf8(pl->path));
			}
			mpd_freeInfoEntity(entity);
		}

		my_finishCommand(conn);

	} while (++i < argc && (ls = charset_to_utf8(argv[i])) != NULL);
	return 0;
}

int cmd_load ( int argc, char ** argv, mpd_Connection * conn )
{
	int i;
	char * sp;
	char * dp;
	mpd_InfoEntity * entity;
	mpd_PlaylistFile * pl;

	for(i=0;i<argc;i++) {
		sp = argv[i];
		while((sp = strchr(sp,' '))) *sp = '_';
	}

	mpd_sendLsInfoCommand(conn,"");
	printErrorAndExit(conn);
	while((entity = mpd_getNextInfoEntity(conn))) {
		if(entity->type==MPD_INFO_ENTITY_TYPE_PLAYLISTFILE) {
			pl = entity->info.playlistFile;
			dp = sp = strdup(charset_from_utf8(pl->path));
			while((sp = strchr(sp,' '))) *sp = '_';
			for(i=0;i<argc;i++) {
				if(strcmp(dp,argv[i])==0)
					strcpy(argv[i], charset_from_utf8(pl->path));
			}
			free(dp);
			mpd_freeInfoEntity(entity);
		}
	}
	my_finishCommand(conn);

	mpd_sendCommandListBegin(conn);
	printErrorAndExit(conn);
	for(i=0;i<argc;i++) {
		printf("loading: %s\n",argv[i]);
		mpd_sendLoadCommand(conn,charset_to_utf8(argv[i]));
		printErrorAndExit(conn);
	}
	mpd_sendCommandListEnd(conn);
	my_finishCommand(conn);

	return 0;
}

static int do_search ( int argc, char ** argv, mpd_Connection * conn, int exact ) 
{
	Constraint *constraints;
	int numconstraints;
	int i;

	if (argc % 2 != 0)
		DIE("arguments must be pairs of search types and queries\n");

	numconstraints = get_constraints(argc, argv, &constraints);
	if (numconstraints < 0)
		return -1;

	mpd_startSearch(conn, exact);

	for (i = 0; i < numconstraints; i++) {
		mpd_addConstraintSearch(conn, constraints[i].type,
		                        charset_to_utf8(constraints[i].query));
	}

	free(constraints);

	mpd_commitSearch(conn);
	printErrorAndExit(conn);

	print_filenames(conn);

	my_finishCommand(conn);

	return 0;
}

int cmd_search ( int argc, char ** argv, mpd_Connection * conn ) 
{
	return do_search(argc, argv, conn, 0);
}

int cmd_find ( int argc, char ** argv, mpd_Connection * conn ) 
{
	return do_search(argc, argv, conn, 1);
}

int cmd_list ( int argc, char ** argv, mpd_Connection * conn ) 
{
	Constraint *constraints;
	int numconstraints = 0;
	int type;
	int i;
	char *tag;

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

	mpd_startFieldSearch(conn, type);

	if (argc > 0) {
		for (i = 0; i < numconstraints; i++) {
			mpd_addConstraintSearch(conn, constraints[i].type,
						charset_to_utf8(constraints[i].query));
		}

		free(constraints);
	}

	mpd_commitSearch(conn);
	printErrorAndExit(conn);

	while ((tag = mpd_getNextTag(conn, type))) {
		printErrorAndExit(conn);
		printf("%s\n", charset_from_utf8(tag));
		free(tag);
	}

	my_finishCommand(conn);

	return 0;
}

int cmd_volume ( int argc, char ** argv, mpd_Connection * conn ) 
{
        struct int_value_change ch;

	if(argc==1) {
                if(!parse_int_value_change(argv[0], &ch))
			DIE("\"%s\" is not an integer\n", argv[0]);
	} else {
		mpd_Status *status;

		status = getStatus(conn);

		printf("volume:%3i%c   \n",status->volume,'%');

		mpd_freeStatus(status);

		return 0;
	}

	if (ch.is_relative)
		mpd_sendVolumeCommand(conn,ch.value);
	else 
		mpd_sendSetvolCommand(conn,ch.value);

	my_finishCommand(conn);
	return 1;
}

int
cmd_pause(mpd_unused int argc, mpd_unused char **argv, mpd_Connection *conn)
{
	mpd_sendPauseCommand(conn,1);
	my_finishCommand(conn);

	return 1;
}

int cmd_repeat ( int argc, char ** argv, mpd_Connection * conn )
{
	int mode;

	if(argc==1) {
		mode = get_boolean(argv[0]);
		if (mode < 0)
			return -1;
	}
	else {
		mpd_Status * status;
		status = getStatus(conn);
		mode = !status->repeat;
		mpd_freeStatus(status);
	}


	mpd_sendRepeatCommand(conn,mode);
	printErrorAndExit(conn);
	my_finishCommand(conn);
	printErrorAndExit(conn);

	return 1;
}

int cmd_random ( int argc, char ** argv, mpd_Connection * conn )
{
	int mode;

	if(argc==1) {
		mode = get_boolean(argv[0]);
		if (mode < 0)
			return -1;
	}
	else {
		mpd_Status * status;
		status = getStatus(conn);
		mode = !status->random;
		mpd_freeStatus(status);
	}

	mpd_sendRandomCommand(conn,mode);
	my_finishCommand(conn);

	return 1;
}

int cmd_crossfade ( int argc, char ** argv, mpd_Connection * conn )
{
	int seconds;

	if(argc==1) {
                if(!parse_int(argv[0], &seconds) || seconds<0)
			DIE("\"%s\" is not 0 or positive integer\n",argv[0]);

		mpd_sendCrossfadeCommand(conn,seconds);
		my_finishCommand(conn);
	}
	else {
		mpd_Status * status;
		status = getStatus(conn);

		printf("crossfade: %i\n",status->crossfade);

		mpd_freeStatus(status);
		printErrorAndExit(conn);
	}
	return 0;
}

int
cmd_version(mpd_unused int argc, mpd_unused char **argv, mpd_Connection *conn)
{
	printf("mpd version: %i.%i.%i\n",conn->version[0],
			conn->version[1],conn->version[2]);
	return 0;
}

int cmd_loadtab ( int argc, char ** argv, mpd_Connection * conn )
{
	mpd_InfoEntity * entity;
	char * sp;
	mpd_PlaylistFile * pl;

	mpd_sendLsInfoCommand(conn,"");
	printErrorAndExit(conn);

	while((entity = mpd_getNextInfoEntity(conn))) {
		if(entity->type==MPD_INFO_ENTITY_TYPE_PLAYLISTFILE) {
			pl = entity->info.playlistFile;
			sp = pl->path;
			while((sp = strchr(sp,' '))) *sp = '_';
			if(argc==1) {
				if(strncmp(pl->path,argv[0],
							strlen(argv[0]))==0) {
					printf("%s\n",
					       charset_from_utf8(pl->path));
				}
			}
		}
		mpd_freeInfoEntity(entity);
	}

	my_finishCommand(conn);
	return 0;
}

int cmd_lstab ( int argc, char ** argv, mpd_Connection * conn )
{
	mpd_InfoEntity * entity;
	mpd_Directory * dir;

	mpd_sendListallCommand(conn,"");
	printErrorAndExit(conn);

	while((entity = mpd_getNextInfoEntity(conn))) {
		if(entity->type==MPD_INFO_ENTITY_TYPE_DIRECTORY) {
			dir = entity->info.directory;
			if(argc==1) {
				if(strncmp(dir->path,argv[0],
							strlen(argv[0]))==0) {
					printf("%s\n",
					       charset_from_utf8(dir->path));
				}
			}
		}
		mpd_freeInfoEntity(entity);
	}

	my_finishCommand(conn);

	return 0;
}

int cmd_tab ( int argc, char ** argv, mpd_Connection * conn )
{
	mpd_InfoEntity * entity;
	struct mpd_song *song;

	mpd_sendListallCommand(conn,"");
	printErrorAndExit(conn);

	while((entity = mpd_getNextInfoEntity(conn))) {
		if(entity->type==MPD_INFO_ENTITY_TYPE_SONG) {
			song = entity->info.song;
			if(argc==1) {
				if(strncmp(song->file,argv[0],
							strlen(argv[0]))==0) {
					printf("%s\n",
					       charset_from_utf8(song->file));
				}
			} else
				printf("%s\n", charset_from_utf8(song->file));
		}
		mpd_freeInfoEntity(entity);
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
cmd_stats(mpd_unused int argc, mpd_unused char **argv, mpd_Connection *conn)
{
	mpd_Stats *stats;
	time_t t;

	mpd_sendStatsCommand(conn);
	stats = mpd_getStats(conn);
	printErrorAndExit(conn);

	if (stats != NULL) {
		t = stats->dbUpdateTime;
		printf("Artists: %6d\n", stats->numberOfArtists);
		printf("Albums:  %6d\n", stats->numberOfAlbums);
		printf("Songs:   %6d\n", stats->numberOfSongs);
		printf("\n");
		printf("Play Time:    %s\n", DHMS(stats->playTime));
		printf("Uptime:       %s\n", DHMS(stats->uptime));
		printf("DB Updated:   %s", ctime(&t));	/* no \n needed */
		printf("DB Play Time: %s\n", DHMS(stats->dbPlayTime));
		mpd_freeStats(stats);
	} else {
		printf("Error getting mpd stats\n");
	}
	return 0;
}

int
cmd_status(mpd_unused  int argc, mpd_unused char **argv, mpd_Connection *conn)
{
	print_status(conn);
	return 0;
}
