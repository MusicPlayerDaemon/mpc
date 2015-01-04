/*
 * music player command (mpc)
 * Copyright (C) 2003-2013 The Music Player Daemon Project
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
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "format.h"
#include "charset.h"

#include <mpd/client.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Reallocate the given string and append the source string.
 */
gcc_malloc
static char *
string_append(char *dest, const char *src, size_t len)
{
	size_t destlen = dest != NULL
		? strlen(dest)
		: 0;

	dest = realloc(dest, destlen + len + 1);
	memcpy(dest + destlen, src, len);
	dest[destlen + len] = '\0';

	return dest;
}

/**
 * Skip the format string until the current group is closed by either
 * '&', '|' or ']' (supports nesting).
 */
gcc_pure
static const char *
skip_format(const char *p)
{
	unsigned stack = 0;

	while (*p != '\0') {
		if (*p == '[')
			stack++;
		else if (*p == '#' && p[1] != '\0')
			/* skip escaped stuff */
			++p;
		else if (stack > 0) {
			if (*p == ']')
				--stack;
		} else if (*p == '&' || *p == '|' || *p == ']')
			break;

		++p;
	}

	return p;
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
	static char buffer[10];
	const char *value;

	if (strcmp(name, "file") == 0)
		value = mpd_song_get_uri(song);
	else if (strcmp(name, "time") == 0) {
		unsigned duration = mpd_song_get_duration(song);

		if (duration > 0) {
			snprintf(buffer, sizeof(buffer), "%d:%02d",
				 duration / 60, duration % 60);
			value = buffer;
		} else
			value = NULL;
	} else if (strcmp(name, "position") == 0) {
		unsigned pos = mpd_song_get_pos(song);
		snprintf(buffer, sizeof(buffer), "%d", pos+1);
		value = buffer;
	} else if (strcmp(name, "id") == 0) {
		snprintf(buffer, sizeof(buffer), "%u", mpd_song_get_id(song));
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

static char *
format_song2(const struct mpd_song *song,
	     const char *format, const char **last)
{
	char *ret = NULL;
	const char *p;
	bool found = false;

	/* we won't mess up format, we promise... */
	for (p = format; *p != '\0';) {
		switch (p[0]) {
		case '|':
			++p;
			if (!found) {
				/* nothing found yet: try the next
				   section */
				free(ret);
				ret = NULL;
			} else
				/* already found a value: skip the
				   next section */
				p = skip_format(p);
			break;

		case '&':
			++p;
			if (!found)
				/* nothing found yet, so skip this
				   section */
				p = skip_format(p);
			else
				/* we found something yet, but it will
				   only be used if the next section
				   also found something, so reset the
				   flag */
				found = false;
			break;

		case '[': {
			char *t = format_song2(song, p + 1, &p);
			if (t != NULL) {
				ret = string_append(ret, t, strlen(t));
				free(t);
				found = true;
			}
		}
			break;

		case ']':
			if (last != NULL)
				*last = p + 1;
			if (!found) {
				free(ret);
				ret = NULL;
			}
			return ret;

		case '\\': {
			/* take care of escape sequences */
			char ltemp;
			switch (p[1]) {
			case 'a':
				ltemp = '\a';
				break;

			case 'b':
				ltemp = '\b';
				break;

			case 't':
				ltemp = '\t';
				break;

			case 'n':
				ltemp = '\n';
				break;

			case 'v':
				ltemp = '\v';
				break;

			case 'f':
				ltemp = '\f';
				break;

			case 'r':
				ltemp = '\r';
				break;

			case '[':
				ltemp = '[';
				break;

			case ']':
				ltemp = ']';
				break;

			default:
				/* unknown escape: copy the
				   backslash */
				ltemp = p[0];
				--p;
				break;
			}

			ret = string_append(ret, &ltemp, 1);
			p += 2;
		}
			break;

		case '%': {
			/* find the extent of this format specifier
			   (stop at \0, ' ', or esc) */
			const char *end = p + 1;
			while (*end >= 'a' && *end <= 'z')
				++end;

			const size_t length = end - p + 1;

			if (*end != '%') {
				ret = string_append(ret, p, length - 1);
				p = end;
				continue;
			}

			char name[32];
			if (length > (int)sizeof(name)) {
				ret = string_append(ret, p, length);
				p = end + 1;
				continue;
			}

			memcpy(name, p + 1, length - 2);
			name[length - 2] = 0;

			const char *value = song_value(song, name);
			size_t value_length;
			if (value != NULL) {
				if (*value != 0)
					found = true;
				value_length = strlen(value);
			} else {
				/* unknown variable: copy verbatim
				   from format string */
				value = p;
				value_length = length;
			}

			ret = string_append(ret, value, value_length);

			/* advance past the specifier */
			p = end + 1;
		}
			break;

		case '#':
			/* let the escape character escape itself */
			if (p[1] != '\0') {
				ret = string_append(ret, p + 1, 1);
				p += 2;
				break;
			}

			/* fall through */

		default:
			/* pass-through non-escaped portions of the format string */
			ret = string_append(ret, p, 1);
			++p;
		}
	}

	if (last != NULL)
		*last = p;
	return ret;
}

char *
format_song(const struct mpd_song *song, const char *format)
{
	return format_song2(song, format, NULL);
}
