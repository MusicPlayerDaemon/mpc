// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef MPC_NEIGHBORS_H
#define MPC_NEIGHBORS_H

struct mpd_connection;

int
cmd_listneighbors(int argc, char **argv, struct mpd_connection *connection);

#endif /* MPC_NEIGHBORS_H */
