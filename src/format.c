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

gcc_malloc
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

/* this is a little ugly... */
char *
format_song(const struct mpd_song *song,
	    const char *format, const char **last)
{
	char *ret = NULL;
	const char *p;
	bool found = false;

	/* we won't mess up format, we promise... */
	for (p = format; *p != '\0';) {
		if (p[0] == '|') {
			++p;
			if (!found) {
				free(ret);
				ret = NULL;
			} else
				p = skip_format(p);
		} else if (p[0] == '&') {
			++p;
			if (!found)
				p = skip_format(p);
			else
				found = false;
		} else if (p[0] == '[') {
			char *t = format_song(song, p + 1, &p);
			if (t != NULL) {
				ret = string_append(ret, t, strlen(t));
				free(t);
				found = true;
			}
		} else if (p[0] == ']') {
			if (last != NULL)
				*last = p + 1;
			if (!found) {
				free(ret);
				ret = NULL;
			}
			return ret;
		} else if (p[0] == '\\') {
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
				ltemp = p[0];
				--p;
				break;
			}

			ret = string_append(ret, &ltemp, 1);
			p += 2;
		} else if (p[0] != '#' && p[0] != '%') {
			/* pass-through non-escaped portions of the format string */
			ret = string_append(ret, p, 1);
			++p;
		} else if (p[0] == '#' && p[1] != '\0') {
			/* let the escape character escape itself */
			ret = string_append(ret, p + 1, 1);
			p += 2;
		} else {
			/* find the extent of this format specifier
			   (stop at \0, ' ', or esc) */
			const char *end = p + 1;
			while (*end >= 'a' && *end <= 'z')
				++end;

			const size_t length = end - p + 1;

			if (*end != '%') {
				ret = string_append(ret, p, length - 1);
				p += length - 1;
				continue;
			}

			char name[32];
			if (length > (int)sizeof(name)) {
				ret = string_append(ret, p, length);
				p += length;
				continue;
			}

			memcpy(name, p + 1, length - 2);
			name[length - 2] = 0;

			const char *temp = song_value(song, name);
			if (temp != NULL) {
				if (*temp != 0)
					found = true;
				ret = string_append(ret, temp, strlen(temp));
			} else
				ret = string_append(ret, p, length);

			/* advance past the specifier */
			p += length;
		}
	}

	if (last != NULL)
		*last = p;
	return ret;
}
