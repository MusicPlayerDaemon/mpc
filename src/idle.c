// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "idle.h"
#include "util.h"

#include <mpd/client.h>

#include <stdbool.h>
#include <stdio.h>

int cmd_idle(int argc, char **argv,
	     struct mpd_connection *connection)
{
	enum mpd_idle idle = 0;

	for (int i = 0; i < argc; ++i) {
		enum mpd_idle parsed = mpd_idle_name_parse(argv[i]);
		if (parsed == 0) {
			fprintf(stderr, "Unrecognized idle event: %s\n",
				argv[i]);
			return 1;
		}

		idle |= parsed;
	}

	idle = idle == 0 ? mpd_run_idle(connection)
		: mpd_run_idle_mask(connection, idle);
	if (idle == 0 &&
	    mpd_connection_get_error(connection) != MPD_ERROR_SUCCESS)
		printErrorAndExit(connection);

	for (unsigned j = 0;; ++j) {
		enum mpd_idle i = 1 << j;
		const char *name = mpd_idle_name(i);

		if (name == NULL)
			break;

		if (idle & i)
			printf("%s\n", name);
	}

	return 0;
}

int
cmd_idleloop(int argc, char **argv, struct mpd_connection *connection)
{
	while (true) {
		int ret = cmd_idle(argc, argv, connection);
		fflush(stdout);
		if (ret != 0)
			return ret;
	}
}
