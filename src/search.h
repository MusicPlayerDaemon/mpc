// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef MPC_SEARCH_H
#define MPC_SEARCH_H

#include <mpd/client.h>

struct mpd_connection;

bool
add_constraints(int argc, char ** argv, struct mpd_connection *conn);

int
cmd_search(int argc, char **argv, struct mpd_connection *conn);

int
cmd_searchadd(int argc, char **argv, struct mpd_connection *conn);

int
cmd_find(int argc, char **argv, struct mpd_connection *conn);

int
cmd_findadd(int argc, char **argv, struct mpd_connection *conn);

int
cmd_searchplaylist(int argc, char **argv, struct mpd_connection *conn);

#endif
