// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef MPC_MESSAGE_H
#define MPC_MESSAGE_H

struct mpd_connection;

int
cmd_channels(int argc, char **argv, struct mpd_connection *connection);

int
cmd_sendmessage(int argc, char **argv, struct mpd_connection *connection);

int
cmd_waitmessage(int argc, char **argv, struct mpd_connection *connection);

int
cmd_subscribe(int argc, char **argv, struct mpd_connection *connection);

#endif /* MPC_MESSAGE_H */
