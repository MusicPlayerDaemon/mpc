// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "status.h"
#include "status_format.h"
#include "util.h"
#include "charset.h"
#include "mpc.h"

#include <mpd/client.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

void
print_status(struct mpd_connection *conn)
{
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

		if (mpd_status_get_state(status) == MPD_STATE_PLAY)
			printf("[playing]");
		else
			printf("[paused] ");

		printf(" #%i/%u %3i:%02i/%i:%02i (%u%%)\n",
		       mpd_status_get_song_pos(status) + 1,
		       mpd_status_get_queue_length(status),
		       mpd_status_get_elapsed_time(status) / 60,
		       mpd_status_get_elapsed_time(status) % 60,
		       mpd_status_get_total_time(status) / 60,
		       mpd_status_get_total_time(status) % 60,
		       elapsed_percent(status));
	}

	if (mpd_status_get_update_id(status) > 0)
		printf("Updating DB (#%u) ...\n",
		       mpd_status_get_update_id(status));

	if (mpd_status_get_volume(status) >= 0)
		printf("volume:%3i%c   ", mpd_status_get_volume(status), '%');
	else {
		printf("volume: n/a   ");
	}

	printf("repeat: ");
	if (mpd_status_get_repeat(status))
		printf("on    ");
	else printf("off   ");

	printf("random: ");
	if (mpd_status_get_random(status))
		printf("on    ");
	else printf("off   ");

	printf("single: ");
	if (mpd_status_get_single_state(status) == MPD_SINGLE_ON)
		printf("on    ");
	else if (mpd_status_get_single_state(status) == MPD_SINGLE_ONESHOT)
		printf("once  ");
	else if (mpd_status_get_single_state(status) == MPD_SINGLE_OFF)
		printf("off   ");

	printf("consume: ");
#if LIBMPDCLIENT_CHECK_VERSION(2,21,0)
	if (mpd_status_get_consume_state(status) == MPD_CONSUME_ON)
		printf("on  \n");
	else if (mpd_status_get_consume_state(status) == MPD_CONSUME_ONESHOT)
		printf("once\n");
	else if (mpd_status_get_consume_state(status) == MPD_CONSUME_OFF)
		printf("off \n");
#else
	if (mpd_status_get_consume(status))
		printf("on \n");
	else printf("off\n");
#endif

	if (mpd_status_get_error(status) != NULL)
		printf("ERROR: %s\n",
		       charset_from_utf8(mpd_status_get_error(status)));

	mpd_status_free(status);

	my_finishCommand(conn);
}

