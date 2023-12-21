// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "neighbors.h"
#include "util.h"
#include "charset.h"
#include "Compiler.h"

#include <mpd/client.h>

#include <stdio.h>

int
cmd_listneighbors(gcc_unused int argc, gcc_unused char **argv,
		  struct mpd_connection *connection)
{
	if (!mpd_send_command(connection, "listneighbors", NULL))
		printErrorAndExit(connection);

	struct mpd_pair *pair;
	while ((pair = mpd_recv_pair_named(connection, "neighbor")) != NULL) {
		printf("%s\n", charset_from_utf8(pair->value));
		mpd_return_pair(connection, pair);
	}

	if (!mpd_response_finish(connection))
		printErrorAndExit(connection);

	return 0;
}
