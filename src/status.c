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

#include "status.h"
#include "libmpdclient.h"
#include "charset.h"
#include "util.h"
#include "mpc.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void print_status (mpd_Connection *conn)
{
	mpd_Status * status;
	mpd_InfoEntity * entity;

	mpd_sendCommandListOkBegin(conn);
	printErrorAndExit(conn);
	mpd_sendStatusCommand(conn);
	printErrorAndExit(conn);
	mpd_sendCurrentSongCommand(conn);
	printErrorAndExit(conn);
	mpd_sendCommandListEnd(conn);
	printErrorAndExit(conn);

	status = mpd_getStatus(conn);
	printErrorAndExit(conn);

	if(status->state == MPD_STATUS_STATE_PLAY || 
			status->state == MPD_STATUS_STATE_PAUSE) 
	{
		float perc;

		mpd_nextListOkCommand(conn);
		printErrorAndExit(conn);
			
		while((entity = mpd_getNextInfoEntity(conn))) {
			struct mpd_song *song = entity->info.song;
			
			if(entity->type!=MPD_INFO_ENTITY_TYPE_SONG) {
				mpd_freeInfoEntity(entity);
				continue;
			}

			pretty_print_song(song);
			printf("\n");

			mpd_freeInfoEntity(entity);

			break;
		}

		printErrorAndExit(conn);

		mpd_finishCommand(conn);
		printErrorAndExit(conn);

		if(status->state==MPD_STATUS_STATE_PLAY) {
			printf("[playing]");
		}
		else printf("[paused] ");

		perc = status->elapsedTime<status->totalTime ?
				100.0*status->elapsedTime/status->totalTime :
				100.0;


		printf(" #%i/%i %3i:%02i/%i:%02i (%.0f%c)\n",
				status->song+1,
				status->playlistLength,
				status->elapsedTime/60,
				status->elapsedTime%60,
				status->totalTime/60,
				status->totalTime%60,
				perc,'%');
	}

	if(status->updatingDb) {
		printf("Updating DB (#%i) ...\n",status->updatingDb);
	}

	if(status->volume!=MPD_STATUS_NO_VOLUME) {
		printf("volume:%3i%c   ",status->volume,'%');
	}
	else {
		printf("volume: n/a   ");
	}

	printf("repeat: ");
	if(status->repeat) printf("on    ");
	else printf("off   ");

	printf("random: ");
	if(status->random) printf("on \n");
	else printf("off\n");

	if (status->error != NULL)
		printf("ERROR: %s\n", charset_from_utf8(status->error));

	mpd_freeStatus(status);
}

