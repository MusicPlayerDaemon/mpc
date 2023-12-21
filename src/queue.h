// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef MPC_QUEUE_H
#define MPC_QUEUE_H

struct mpd_connection;

int
cmd_clear(int argc, char **argv, struct mpd_connection *conn);

int
cmd_shuffle(int argc, char **argv, struct mpd_connection *conn);

int
cmd_add(int argc, char **argv, struct mpd_connection *conn);

int
cmd_crop(int argc, char **argv, struct mpd_connection *conn);

int
cmd_del(int argc, char **argv, struct mpd_connection *conn);

int
cmd_playlist(int argc, char **argv, struct mpd_connection *conn);

int
cmd_insert(int argc, char **argv, struct mpd_connection *conn);

int
cmd_prio(int argc, char **argv, struct mpd_connection *conn);

#endif /* COMMAND_H */
