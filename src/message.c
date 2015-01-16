/*
 * music player command (mpc)
 * Copyright (C) 2003-2015 The Music Player Daemon Project
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

#include "message.h"
#include "util.h"
#include "charset.h"
#include "Compiler.h"

#include <mpd/client.h>

#if LIBMPDCLIENT_CHECK_VERSION(2,5,0) && !LIBMPDCLIENT_CHECK_VERSION(2,5,1)
#include <mpd/message.h>
#endif

#include <stdio.h>

#if LIBMPDCLIENT_CHECK_VERSION(2,5,0)

int
cmd_channels(gcc_unused int argc, gcc_unused char **argv,
	     struct mpd_connection *connection)
{
	if (!mpd_send_channels(connection))
		printErrorAndExit(connection);

	struct mpd_pair *pair;
	while ((pair = mpd_recv_channel_pair(connection)) != NULL) {
		/* channel names may only contain certain ASCII characters */
		printf("%s\n", pair->value);
		mpd_return_pair(connection, pair);
	}

	my_finishCommand(connection);
	return 0;
}

int
cmd_sendmessage(gcc_unused int argc, char **argv,
		struct mpd_connection *connection)
{
	const char *text = charset_to_utf8(argv[1]);
	if (!mpd_run_send_message(connection, argv[0], text))
		printErrorAndExit(connection);

	return 0;
}

int
cmd_waitmessage(gcc_unused int argc, char **argv,
		struct mpd_connection *connection)
{
	if (!mpd_run_subscribe(connection, argv[0]) ||
	    !mpd_run_idle_mask(connection, MPD_IDLE_MESSAGE) ||
	    !mpd_send_read_messages(connection))
		printErrorAndExit(connection);

	struct mpd_message *message;
	while ((message = mpd_recv_message(connection)) != NULL) {
		const char *tmp = mpd_message_get_text(message);
		printf("%s\n", charset_from_utf8(tmp));
		mpd_message_free(message);
	}

	my_finishCommand(connection);
	return 0;
}

int
cmd_subscribe(gcc_unused int argc, char **argv,
	      struct mpd_connection *connection)
{
	if (!mpd_run_subscribe(connection, argv[0]))
		printErrorAndExit(connection);

	while (true) {
		if (!mpd_run_idle_mask(connection, MPD_IDLE_MESSAGE) ||
		    !mpd_send_read_messages(connection))
			printErrorAndExit(connection);

		struct mpd_message *message;
		while ((message = mpd_recv_message(connection)) != NULL) {
			const char *tmp = mpd_message_get_text(message);
			printf("%s\n", charset_from_utf8(tmp));
			mpd_message_free(message);
		}

		my_finishCommand(connection);
	}
}

#endif /* libmpdclient 2.5 */
