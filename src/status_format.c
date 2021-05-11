/*
 * music player command (mpc)
 * Copyright 2003-2020 The Music Player Daemon Project
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
 * with this program; if offt, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "status_format.h"
#include "format.h"
#include "charset.h"
#include "util.h"

#include <mpd/client.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct mpd_status *
get_status_from_conn(struct mpd_connection *conn) {
    if (!mpd_command_list_begin(conn, true) ||
	    !mpd_send_status(conn) ||
	    !mpd_send_current_song(conn) ||
	    !mpd_command_list_end(conn))
	printErrorAndExit(conn);

    struct mpd_status *status = mpd_recv_status(conn);
    if (status == NULL)
	printErrorAndExit(conn);
    return status;
}

unsigned
elapsed_percent(const struct mpd_status *status)
{
	unsigned total = mpd_status_get_total_time(status);
	if (total == 0)
		return 0;

	unsigned elapsed = mpd_status_get_elapsed_time(status);
	if (elapsed >= total)
		return 100;

	return (elapsed * 100) / total;
}

/**
 * Extract an attribute from a status object
 *
 * @param status the status object
 * @param name the attribute name
 * @return the attribute value; NULL if the attribute name is invalid
 */
gcc_pure
static const char *
status_value(const struct mpd_status *status, const char *name)
{
	static char buffer[40];

	if (strcmp(name, "totaltime") == 0) {
		unsigned duration = mpd_status_get_total_time(status);
		snprintf(buffer, sizeof(buffer), "%u:%02u",
			duration / 60, duration % 60);
	} else if (strcmp(name, "songpos") == 0) {
		int song_pos = mpd_status_get_song_pos(status) + 1;
		snprintf(buffer, sizeof(buffer), "%i", song_pos);
	} else if (strcmp(name, "length") == 0) {
		unsigned length = mpd_status_get_queue_length(status);
		snprintf(buffer, sizeof(buffer), "%i", length);
	} else if (strcmp(name, "currenttime") == 0) {
		unsigned elasped = mpd_status_get_elapsed_time(status);
		snprintf(buffer, sizeof(buffer), "%u:%02u",
			elasped / 60, elasped % 60);
	} else if (strcmp(name, "percenttime") == 0) {
		snprintf(buffer, sizeof(buffer), "%u", elapsed_percent(status));
	} else if (strcmp(name, "state") == 0) {
		if (mpd_status_get_state(status) == MPD_STATE_PLAY) {
			snprintf(buffer, sizeof(buffer), "playing");
		} else {
			snprintf(buffer, sizeof(buffer), "paused");
		}
	} else if (strcmp(name, "volume") == 0) {
		int vol = mpd_status_get_volume(status);
		snprintf(buffer, sizeof(buffer), "%3i%c", vol, '%');
	} else if (strcmp(name, "repeat") == 0) {
		if (mpd_status_get_repeat(status)) {
			snprintf(buffer, sizeof(buffer), "on");
		} else {
			snprintf(buffer, sizeof(buffer), "off");
		}
	} else if (strcmp(name, "random") == 0) {
		if (mpd_status_get_random(status)) {
			snprintf(buffer, sizeof(buffer), "on");
		} else {
			snprintf(buffer, sizeof(buffer), "off");
		}
	} else if (strcmp(name, "single") == 0) {
		if (mpd_status_get_single(status)) {
			snprintf(buffer, sizeof(buffer), "on");
		} else {
			snprintf(buffer, sizeof(buffer), "off");
		}
	} else if (strcmp(name, "consume") == 0) {
		if (mpd_status_get_consume(status)) {
			snprintf(buffer, sizeof(buffer), "on");
		} else {
			snprintf(buffer, sizeof(buffer), "off");
		}
	}
	else { return NULL; }
	return charset_from_utf8(buffer);
}

static const char *
status_getter(const void *object, const char *name)
{
	return status_value((const struct mpd_status *)object, name);
}

char *
format_status(const struct mpd_status *status, const char *format)
{
	return format_object(format, status, status_getter);
}
