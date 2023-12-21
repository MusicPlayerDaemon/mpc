// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "song_format.h"
#include "audio_format.h"
#include "format.h"
#include "charset.h"

#include <mpd/client.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char *
format_mtime(char *buffer, size_t buffer_size,
	     const struct mpd_song *song, const char *format)
{
	time_t t = mpd_song_get_last_modified(song);
	if (t == 0)
		return NULL;

	struct tm tm;
#ifdef _WIN32
	tm = *localtime(&t);
#else
	localtime_r(&t, &tm);
#endif

	strftime(buffer, buffer_size, format, &tm);
	return buffer;
}

/**
 * Extract an attribute from a song object.
 *
 * @param song the song object
 * @param name the attribute name
 * @return the attribute value; NULL if the attribute name is invalid;
 * an empty string if the attribute name is valid, but not present in
 * the song
 */
gcc_pure
static const char *
song_value(const struct mpd_song *song, const char *name)
{
	static char buffer[40];
	const char *value;

	if (strcmp(name, "file") == 0)
		value = mpd_song_get_uri(song);
	else if (strcmp(name, "time") == 0) {
		unsigned duration = mpd_song_get_duration(song);

		if (duration > 0) {
			snprintf(buffer, sizeof(buffer), "%u:%02u",
				 duration / 60, duration % 60);
			value = buffer;
		} else
			value = NULL;
	} else if (strcmp(name, "position") == 0) {
		unsigned pos = mpd_song_get_pos(song);
		snprintf(buffer, sizeof(buffer), "%u", pos+1);
		value = buffer;
	} else if (strcmp(name, "id") == 0) {
		snprintf(buffer, sizeof(buffer), "%u", mpd_song_get_id(song));
		value = buffer;
	} else if (strcmp(name, "prio") == 0) {
		snprintf(buffer, sizeof(buffer), "%u",
			 mpd_song_get_prio(song));
		value = buffer;
	} else if (strcmp(name, "mtime") == 0) {
		value = format_mtime(buffer, sizeof(buffer), song, "%c");
	} else if (strcmp(name, "mdate") == 0) {
		value = format_mtime(buffer, sizeof(buffer), song, "%x");
	} else if (strcmp(name, "audioformat") == 0) {
		const struct mpd_audio_format *audio_format = mpd_song_get_audio_format(song);
		if (audio_format == NULL)
			return NULL;

		format_audio_format(buffer, sizeof(buffer), audio_format);
		value = buffer;
	} else {
		enum mpd_tag_type tag_type = mpd_tag_name_iparse(name);
		if (tag_type == MPD_TAG_UNKNOWN)
			return NULL;

		value = mpd_song_get_tag(song, tag_type, 0);
	}

	if (value != NULL)
		value = charset_from_utf8(value);
	else
		value = "";

	return value;
}

static const char *
song_getter(const void *object, const char *name)
{
	return song_value((const struct mpd_song *)object, name);
}

char *
format_song(const struct mpd_song *song, const char *format)
{
	return format_object(format, song, song_getter);
}
