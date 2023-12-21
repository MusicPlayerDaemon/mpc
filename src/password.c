// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "password.h"
#include "util.h"

#include <mpd/client.h>

void
send_password(const char *password, struct mpd_connection *conn)
{
	if (!mpd_run_password(conn, password))
		printErrorAndExit(conn);
}

