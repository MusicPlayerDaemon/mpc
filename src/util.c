/* music player command (mpc)
 * Copyright (C) 2003-2008 Warren Dukes <warren.dukes@gmail.com>,
				Eric Wong <normalperson@yhbt.net>,
				Daniel Brown <danb@cs.utexas.edu>
 * Copyright (C) 2008-2010 Max Kellermann <max@duempel.org>
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

#include "util.h"
#include "charset.h"
#include "list.h"
#include "options.h"

#include <mpd/client.h>

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <sys/param.h>

void
printErrorAndExit(struct mpd_connection *conn)
{
	const char *message;

	assert(mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS);

	message = mpd_connection_get_error_message(conn);
	if (mpd_connection_get_error(conn) == MPD_ERROR_SERVER)
		/* messages received from the server are UTF-8; the
		   rest is either US-ASCII or locale */
		message = charset_from_utf8(message);

	fprintf(stderr, "error: %s\n", message);
	mpd_connection_free(conn);
	exit(EXIT_FAILURE);
}

int stdinToArgArray(char *** array)
{
	List * list = makeList(NULL);
	ListNode * node;
	char buffer[4096];
	int size;
	int i;
	char * sp;

	while(fgets(buffer, sizeof(buffer), stdin)) {
		if((sp = strchr(buffer,'\n'))) *sp = '\0';
		insertInListWithoutKey(list,strdup(buffer));
	}

	size = list->numberOfNodes;
	*array = malloc((sizeof(char *))*size);
	i = 0;
	node = list->firstNode;
	while(node) {
		(*array)[i++] = (char *)node->data;
		node = node->nextNode;
	}
	assert(i==size);

	freeList(list);

	return size;
}

void free_pipe_array (int max, char ** array)
{
	int i;
	for ( i=0 ;i<max; ++i) free(array[i]);
}

int get_boolean (const char * arg)
{
	int i;
	static const struct _bool_table {
		const char * on;
		const char * off;
	} bool_table [] = {
		{ "on", "off" },
		{ "1", "0" },
		{ "true", "false" },
		{ "yes", "no" },
		{ .on = NULL }
	};

	for (i=0; bool_table[i].on; ++i) {
		if (! strcasecmp(arg,bool_table[i].on))
			return 1;
		else if (! strcasecmp(arg,bool_table[i].off))
			return 0;
	}
	
	fprintf(stderr,"\"%s\" is not a boolean value: <",arg);
	
	for (i=0; bool_table[i].on; ++i) {
		fprintf(stderr,"%s|%s%s", bool_table[i].on,
			bool_table[i].off,
			( bool_table[i+1].off ? "|" : ">\n"));
	}  
	return -1;
}

/* note - return value is success; the parsed int itself is in ret */

int parse_int(const char * str, int * ret)
{
        char * test;
        int temp;

        temp = strtol(str, &test, 10);

        if(*test != '\0')
                return 0; /* failure */

        *ret = temp;
        return 1; /* success */
}

/* note - return value is success; the parsed float itself is in ret */

int parse_float(const char * str, float * ret)
{
	char * test;
	float temp;

	temp = strtof(str, &test);

	if(*test != '\0')
		return 0; /* failure */

	*ret = temp;
	return 1; /* success */
}

/* note - simply strips number out of formatting; does not -1 or +1 or change
 * the number in any other way for that matter */
int parse_songnum(const char * str, int * ret)
{
        int song;
        char * endptr;

        if(!str)
                return 0;
        if(*str == '#')
                str++;

        song = strtol(str, &endptr, 10);

        if(str == endptr || (*endptr != ')' && *endptr != '\0') || song < 0)
                return 0;

        *ret = song;

        return 1;
}

int parse_int_value_change(const char * str, struct int_value_change * ret)
{
        int len;
        int change;
        int relative = 0;

        len = strlen(str);

        if(len < 1)
                return 0;

        if(*str == '+')
                relative = 1;
        else if(*str == '-')
                relative = -1;

        if(!parse_int(str, &change))
                return 0;

        ret->value = change;
        ret->is_relative = (relative != 0);

        return 1;
}

static char * appendToString(char * dest, const char * src, int len) {
	int destlen;

	if(dest == NULL) {
		dest = malloc(len+1);
		memset(dest, 0, len+1);
		destlen = 0;
	}
	else {
		destlen = strlen(dest);
		dest = realloc(dest, destlen+len+1);
	}

	memcpy(dest+destlen, src, len);
	dest[destlen+len] = '\0';

	return dest;
}

static const char * skipFormatting(const char * p) {
	int stack = 0;
		
	while (*p != '\0') {
		if(*p == '[') stack++;
		else if(*p == '#' && p[1] != '\0') {
			/* skip escaped stuff */
			++p;
		}
		else if(stack) {
			if(*p == ']') stack--;
		}
		else {
			if(*p == '&' || *p == '|' || *p == ']') {
				break;
			}
		}
		++p;
	}

	return p;
}

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
static char *
songToFormatedString(const struct mpd_song *song,
		     const char *format, const char ** last)
{
	char * ret = NULL;
	const char *p, *end;
	const char *temp;
	char name[32];
	int length;
	int found = 0;

	/* we won't mess up format, we promise... */
	for (p = format; *p != '\0'; )
	{
		if (p[0] == '|') {
			++p;
			if(!found) {
				if(ret) {
					free(ret);
					ret = NULL;
				}
			}
			else {
				p = skipFormatting(p);
			}
			continue;
		}
		
		if (p[0] == '&') {
			++p;
			if(found == 0) {
				p = skipFormatting(p);
			}
			else {
				found = 0;
			}
			continue;
		}
		
		if (p[0] == '[')
		{
			char *t = songToFormatedString(song, p+1, &p);
			if(t != NULL) {
				ret = appendToString(ret, t, strlen(t));
				free(t);
				found = 1;
			}
			continue;
		}

		if (p[0] == ']')
		{
			if(last) *last = p+1;
			if(!found && ret) {
				free(ret);
				ret = NULL;
			}
			return ret;
		}

		/* take care of escape sequences */
		while (p[0] == '\\')
		{
			char ltemp;
			switch (p[1]) 
			{
				case 'a':	ltemp = '\a'; break;
				case 'b':	ltemp = '\b'; break;
				case 't':	ltemp = '\t'; break;
				case 'n':	ltemp = '\n'; break;
				case 'v':	ltemp = '\v'; break;
				case 'f':	ltemp = '\f'; break;
				case 'r':	ltemp = '\r'; break;
				default:	ltemp = p[0]; p-=1; break;
			}
			ret = appendToString(ret, &ltemp, 1);
			p+=2;
		}

		/* pass-through non-escaped portions of the format string */
		if (p[0] != '#' && p[0] != '%')
		{
			ret = appendToString(ret, p, 1);
			++p;
			continue;
		}

		/* let the escape character escape itself */
		if (p[0] == '#' && p[1] != '\0')
		{
			ret = appendToString(ret, p+1, 1);
			p+=2;
			continue;
		}

		/* advance past the esc character */

		/* find the extent of this format specifier (stop at \0, ' ', or esc) */
		temp = NULL;

		end  = p+1;
		while(*end >= 'a' && *end <= 'z')
		{
			end++;
		}
		length = end - p + 1;

		if (*end != '%') {
			ret = appendToString(ret, p, length - 1);
			p += length - 1;
			continue;
		}

		if (length > (int)sizeof(name)) {
			ret = appendToString(ret, p, length);
			p += length;
			continue;
		}

		memcpy(name, p + 1, length - 2);
		name[length - 2] = 0;

		temp = song_value(song, name);
		if (temp != NULL) {
			if (*temp != 0)
				found = 1;
			ret = appendToString(ret, temp, strlen(temp));
		} else
			ret = appendToString(ret, p, length);

		/* advance past the specifier */
		p += length;
	}

	if(last) *last = p;
	return ret;
}

static void
print_formatted_song(const struct mpd_song *song, const char * format)
{
	char * str = songToFormatedString(song, format, NULL);

	if(str) {
		printf("%s", str);
		free(str);
	}
}

void
pretty_print_song(const struct mpd_song *song)
{
	print_formatted_song(song, options.format);
}

void
print_entity_list(struct mpd_connection *c, enum mpd_entity_type filter_type)
{
	struct mpd_entity *entity;
	while ((entity = mpd_recv_entity(c)) != NULL) {
		const struct mpd_directory *dir;
		const struct mpd_song *song;
		const struct mpd_playlist *playlist;

		enum mpd_entity_type type = mpd_entity_get_type(entity);
		if (filter_type != MPD_ENTITY_TYPE_UNKNOWN &&
		    type != filter_type)
			type = MPD_ENTITY_TYPE_UNKNOWN;

		switch (type) {
		case MPD_ENTITY_TYPE_UNKNOWN:
			break;

		case MPD_ENTITY_TYPE_DIRECTORY:
			dir = mpd_entity_get_directory(entity);
			printf("%s\n", charset_from_utf8(mpd_directory_get_path(dir)));
			break;

		case MPD_ENTITY_TYPE_SONG:
			song = mpd_entity_get_song(entity);
			if (options.custom_format) {
				pretty_print_song(song);
				puts("");
			} else
				printf("%s\n", charset_from_utf8(mpd_song_get_uri(song)));
			break;

		case MPD_ENTITY_TYPE_PLAYLIST:
			playlist = mpd_entity_get_playlist(entity);
			printf("%s\n", charset_from_utf8(mpd_playlist_get_path(playlist)));
			break;
		}

		mpd_entity_free(entity);
	}
}

void
print_filenames(struct mpd_connection *conn)
{
	struct mpd_song *song;

	while ((song = mpd_recv_song(conn)) != NULL) {
		printf("%s\n", charset_from_utf8(mpd_song_get_uri(song)));
		mpd_song_free(song);
	}

	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
		printErrorAndExit(conn);
}
