// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef MPC_IDLE_H
#define MPC_IDLE_H

struct mpd_connection;

int
cmd_idle(int argc, char **argv, struct mpd_connection *connection);

int
cmd_idleloop(int argc, char **argv, struct mpd_connection *connection);

#endif
