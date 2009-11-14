/*
 * Copyright (C) 2003-2009 The Music Player Daemon Project
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

#include "sticker.h"
#include "util.h"

#include <mpd/client.h>

#include <string.h>
#include <stdio.h>

#if defined(LIBMPDCLIENT_CHECK_VERSION) && LIBMPDCLIENT_CHECK_VERSION(2,1,0)

static void my_finishCommand(struct mpd_connection *conn) {
	if (!mpd_response_finish(conn))
		printErrorAndExit(conn);
}

int
cmd_sticker(int argc, char **argv, struct mpd_connection *conn)
{
	if(!strcmp(argv[1], "set"))
	{
		if(argc < 4)
		{
			fputs("syntax: sticker <uri> set <key> <value>\n", stderr);
			return 0;
		}

		mpd_sticker_song_set(conn, argv[0], argv[2], argv[3]);
		my_finishCommand(conn);
		return 0;
	}
	else if(!strcmp(argv[1], "get"))
	{
		struct mpd_sticker* value;

		if(argc < 3)
		{
			fputs("syntax: sticker <uri> get <key>\n", stderr);
			return 0;
		}

		value = mpd_sticker_song_get(conn, argv[0], argv[2]);
		if(value)
		{
			printf("%s: %s\n", mpd_sticker_get_name(value), mpd_sticker_get_value(value));
			mpd_sticker_free(value);
		}
		my_finishCommand(conn);
	}
	else if(!strcmp(argv[1], "find"))
	{
		struct mpd_sticker* sticker;

		if(argc < 3)
		{
			fputs("syntax: sticker <dir> find <key>\n", stderr);
			return 0;
		}

		sticker = mpd_sticker_song_find(conn, argv[0], argv[2]);
		while(sticker)
		{
			printf("%s: %s=%s\n", mpd_sticker_get_uri(sticker), mpd_sticker_get_name(sticker),
			                      mpd_sticker_get_value(sticker));
			sticker = mpd_sticker_free(sticker);
		}
		my_finishCommand(conn);
	}
	else if(!strncmp(argv[1], "del", 3))
	{
		if(argc < 2)
		{
			fputs("syntax: sticker <uri> delete [key]\n", stderr);
			return 0;
		}

		mpd_sticker_song_delete(conn, argv[0], argc > 2 ? argv[2] : NULL);
		my_finishCommand(conn);
	}
	else if(!strcmp(argv[1], "list"))
	{
		struct mpd_sticker* sticker;

		if(argc < 2)
		{
			fputs("syntax: sticker <uri> list\n", stderr);
			return 0;
		}

		sticker = mpd_sticker_song_list(conn, argv[0]);
		while(sticker)
		{
			printf("%s: %s\n", mpd_sticker_get_name(sticker), mpd_sticker_get_value(sticker));
			sticker = mpd_sticker_free(sticker);
		}
		my_finishCommand(conn);
	}
	else
		fputs("error: unknown command.\n", stderr);

	return 0;
}

#endif /* libmpdclient 2.1 */
