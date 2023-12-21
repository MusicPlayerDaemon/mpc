// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "status_format.h"
#include "audio_format.h"
#include "format.h"
#include "charset.h"
#include "util.h"

#include <mpd/client.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
		sprintf(buffer, "%3u%c", elapsed_percent(status), '%');
	} else if (strcmp(name, "state") == 0) {
		if (mpd_status_get_state(status) == MPD_STATE_PLAY) {
			return "playing";
		} else if (mpd_status_get_state(status) == MPD_STATE_PAUSE) {
			return "paused";
		} else if (mpd_status_get_state(status) == MPD_STATE_STOP) {
			return "stopped";
		} else {
			return NULL;
		}
	} else if (strcmp(name, "volume") == 0) {
		sprintf(buffer, "%3i%c", mpd_status_get_volume(status), '%');
	} else if (strcmp(name, "repeat") == 0) {
		if (mpd_status_get_repeat(status)) {
		    return "on";
		} else {
		    return "off";
		}
	} else if (strcmp(name, "random") == 0) {
		if (mpd_status_get_random(status)) {
		    return "on";
		} else {
		    return "off";
		}
	} else if (strcmp(name, "single") == 0) {
		if (mpd_status_get_single_state(status) == MPD_SINGLE_ON) {
			return "on";
		} else if (mpd_status_get_single_state(status) == MPD_SINGLE_ONESHOT) {
			return "once";
		} else if (mpd_status_get_single_state(status) == MPD_SINGLE_OFF) {
			return "off";
		}
	} else if (strcmp(name, "consume") == 0) {
		if (mpd_status_get_consume(status)) {
		    return "on";
		} else {
		    return "off";
		}
	} else if (strcmp(name, "kbitrate") == 0) {
		sprintf(buffer, "%u", mpd_status_get_kbit_rate(status));
	} else if (strcmp(name, "audioformat") == 0) {
		const struct mpd_audio_format *af = mpd_status_get_audio_format(status);
		if (af != NULL) {
			format_audio_format(buffer, sizeof(buffer), af);
		} else {
			return NULL;
		}
	} else if (strcmp(name, "samplerate") == 0) {
		const struct mpd_audio_format *af = mpd_status_get_audio_format(status);
		if (af != NULL) {
			sprintf(buffer, "%u", af->sample_rate);
		} else {
			return NULL;
		}
	} else if (strcmp(name, "bits") == 0) {
		const struct mpd_audio_format *af = mpd_status_get_audio_format(status);
		if (af != NULL) {
			if (af->bits == MPD_SAMPLE_FORMAT_FLOAT)
				strcpy(buffer, "f");
			else if (af->bits == MPD_SAMPLE_FORMAT_DSD)
				strcpy(buffer, "dsd");
			else
				sprintf(buffer, "%u", af->bits);
		} else {
			return NULL;
		}
	} else if (strcmp(name, "channels") == 0) {
		const struct mpd_audio_format *af = mpd_status_get_audio_format(status);
		if (af != NULL) {
			sprintf(buffer, "%u", af->channels);
		} else {
			return NULL;
		}
	}
	else { return NULL; }
	return buffer;
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
