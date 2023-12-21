// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef MPC_BINARY_H
#define MPC_BINARY_H

struct mpd_connection;

int
cmd_albumart(int argc, char **argv, struct mpd_connection *connection);

int
cmd_readpicture(int argc, char **argv, struct mpd_connection *connection);

#endif /* MPC_BINARY_H */
