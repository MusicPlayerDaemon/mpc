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

/* helper: formats a time_t into buffer or returns NULL on t==0 */
static const char *
format_time(char *buffer, size_t buffer_size,
			time_t t, const char *format)
{
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

/* table of song time-fields */
static const struct {
	const char *key;
	time_t     (*getter)(const struct mpd_song *);
	const char *fmt;
} time_fields[] = {
	{ "mtime", mpd_song_get_last_modified, "%c" },
	{ "mdate", mpd_song_get_last_modified, "%x" },
	{ "atime", mpd_song_get_added,         "%c" },
	{ "adate", mpd_song_get_added,         "%x" },
};

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

	if (strcmp(name, "file") == 0) {
		value = mpd_song_get_uri(song);
		return value ? charset_from_utf8(value) : "";
	}

	if (strcmp(name, "time") == 0) {
		unsigned duration = mpd_song_get_duration(song);
		if (duration > 0) {
			snprintf(buffer, sizeof buffer, "%u:%02u",
					 duration / 60, duration % 60);
			return charset_from_utf8(buffer);
		}
		return "";
	}

	if (strcmp(name, "position") == 0) {
		unsigned pos = mpd_song_get_pos(song);
		snprintf(buffer, sizeof buffer, "%u", pos + 1);
		return charset_from_utf8(buffer);
	}

	if (strcmp(name, "id") == 0) {
		snprintf(buffer, sizeof buffer, "%u",
				 mpd_song_get_id(song));
		return charset_from_utf8(buffer);
	}

	if (strcmp(name, "prio") == 0) {
		snprintf(buffer, sizeof buffer, "%u",
				 mpd_song_get_prio(song));
		return charset_from_utf8(buffer);
	}

	for (size_t i = 0; i < sizeof time_fields / sizeof *time_fields; i++) {
		if (strcmp(name, time_fields[i].key) == 0) {
			value = format_time(buffer, sizeof buffer,
							  time_fields[i].getter(song),
							  time_fields[i].fmt);
			return value ? charset_from_utf8(value) : "";
		}
	}

	if (strcmp(name, "audioformat") == 0) {
		const struct mpd_audio_format *af =
			mpd_song_get_audio_format(song);
		if (af == NULL)
			return NULL;
		format_audio_format(buffer, sizeof buffer, af);
		return charset_from_utf8(buffer);
	}

	/* generic tag lookup */
	{
		enum mpd_tag_type tag = mpd_tag_name_iparse(name);
		if (tag == MPD_TAG_UNKNOWN)
			return NULL;
		value = mpd_song_get_tag(song, tag, 0);
		return value ? charset_from_utf8(value) : "";
	}
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
