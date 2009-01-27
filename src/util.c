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

#include "util.h"
#include "libmpdclient.h"
#include "charset.h"
#include "list.h"
#include "mpc.h"
#include "options.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <sys/param.h>

void printErrorAndExit(mpd_Connection * conn)
{
	if(conn->error) {
		fprintf(stderr,"error: %s\n",charset_from_utf8(conn->errorStr));
		mpd_closeConnection(conn);
		exit(EXIT_FAILURE);
	}
}

int stdinToArgArray(char *** array)
{
	List * list = makeList(NULL);
	ListNode * node;
	char buffer[STRING_LENGTH+1];
	int size;
	int i;
	char * sp;

	while(fgets(buffer,STRING_LENGTH,stdin)) {
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

int get_search_type(const char * arg)
{
	int i;

	for (i = 0; i < MPD_TAG_NUM_OF_ITEM_TYPES; i++) {
		if (strcasecmp(mpdTagItemKeys[i], arg) == 0)
			return i;
	}

	fprintf(stderr, "\"%s\" is not a valid search type: <", arg);

	for (i = 0; i < MPD_TAG_NUM_OF_ITEM_TYPES; i++) {
		fprintf(stderr, "%c%s%s", tolower(mpdTagItemKeys[i][0]),
		                          mpdTagItemKeys[i]+1,
		                          (mpdTagItemKeys[i+1] ? "|" : ">\n"));
	}

	return -1;
}

int get_constraints(int argc, char **argv, Constraint **constraints)
{
	int numconstraints = 0;
	int type;
	int i;

	assert(argc > 0 && argc % 2 == 0);

	*constraints = malloc(sizeof(Constraint)*argc/2);

	for (i = 0; i < argc; i += 2) {
		type = get_search_type(argv[i]);
		if (type < 0) {
			free(*constraints);
			return -1;
		}

		(*constraints)[numconstraints].type = type;
		(*constraints)[numconstraints].query = argv[i+1];
		numconstraints++;
	}

	return numconstraints;
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
		if(*p == '#' && p[1] != '\0') {
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

/* this is a little ugly... */
static char *
songToFormatedString(struct mpd_song *song,
		     const char *format, const char ** last)
{
	char * ret = NULL;
	const char *p, *end;
	const char *temp;
	int length;
	int found = 0;
	int labelFound = 0;

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

		labelFound = 0;

		if (*end != '%') {
			length--;
		} else if (strncmp("%file%", p, length) == 0) {
			temp = charset_from_utf8(song->file);
		} else if (strncmp("%artist%", p, length) == 0) {
			labelFound = 1;
			temp = song->artist ? charset_from_utf8(song->artist) : NULL;
		} else if (strncmp("%album%", p, length) == 0) {
			labelFound = 1;
			temp = song->album ? charset_from_utf8(song->album) : NULL;
		} else if (strncmp("%track%", p, length) == 0) {
			labelFound = 1;
			temp = song->track ? charset_from_utf8(song->track) : NULL;
		} else if (strncmp("%title%", p, length) == 0) {
			labelFound = 1;
			temp = song->title ? charset_from_utf8(song->title) : NULL;
		} else if (strncmp("%name%", p, length) == 0) {
			labelFound = 1;
			temp = song->name ? charset_from_utf8(song->name) : NULL;
		} else if (strncmp("%date%", p, length) == 0) {
			labelFound = 1;
			temp = song->date ? charset_from_utf8(song->date) : NULL;
		} else if (strncmp("%genre%", p, length) == 0) {
			labelFound = 1;
			temp = song->genre ? charset_from_utf8(song->genre) : NULL;
		} else if (strncmp("%composer%", p, length) == 0) {
			labelFound = 1;
			temp = song->composer ? charset_from_utf8(song->composer) : NULL;
		} else if (strncmp("%performer%", p, length) == 0) {
			labelFound = 1;
			temp = song->performer ? charset_from_utf8(song->performer) : NULL;
		} else if (strncmp("%disc%", p, length) == 0) {
			labelFound = 1;
			temp = song->disc ? charset_from_utf8(song->disc) : NULL;
		} else if (strncmp("%comment%", p, length) == 0) {
			labelFound = 1;
			temp = song->comment ? charset_from_utf8(song->comment) : NULL;
		} else if (strncmp("%time%", p, length) == 0) {
			labelFound = 1;
			if (song->time != MPD_SONG_NO_TIME) {
				char s[10];
				snprintf(s, 9, "%d:%02d", song->time / 60, 
				                          song->time % 60);
				/* nasty hack to use static buffer */
				temp = charset_from_utf8(s);
			}
		}

		if( temp == NULL && !labelFound ) {
			ret = appendToString(ret, p, length);
		}
		else if( temp != NULL ) {
			found = 1;
			ret = appendToString(ret, temp, strlen(temp));
		}

		/* advance past the specifier */
		p += length;
	}

	if(last) *last = p;
	return ret;
}

static void print_formatted_song(struct mpd_song *song, const char * format)
{
	char * str = songToFormatedString(song, format, NULL);

	if(str) {
		printf("%s", str);
		free(str);
	}
}

#define DEFAULT_FORMAT "[%name%: &[%artist% - ]%title%]|%name%|[%artist% - ]%title%|%file%"

void pretty_print_song(struct mpd_song *song)
{
	/* was a format string specified? */
	if (get_option("format")->set)
	{
		print_formatted_song(song, get_option("format")->value);
	}
	/* just do something pretty */
	else
	{
		print_formatted_song(song, DEFAULT_FORMAT);
	}
}

void print_filenames(mpd_Connection *conn)
{
	mpd_InfoEntity *entity;

	while ((entity = mpd_getNextInfoEntity(conn))) {
		printErrorAndExit(conn);
		if (entity->type == MPD_INFO_ENTITY_TYPE_SONG)
			printf("%s\n", charset_from_utf8(entity->info.song->file));
		mpd_freeInfoEntity(entity);
	}
}
