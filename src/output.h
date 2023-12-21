// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef OUTPUT_H
#define OUTPUT_H

struct mpd_connection;

int cmd_enable( int argc, char **argv, struct mpd_connection *conn);
int cmd_disable( int argc, char **argv, struct mpd_connection *conn);

int
cmd_toggle_output(int argc, char **argv, struct mpd_connection *conn);

int cmd_outputs(int argc, char **argv, struct mpd_connection *conn);

int
cmd_outputset(int argc, char **argv, struct mpd_connection *conn);

int
cmd_moveoutput(int argc, char **argv, struct mpd_connection *conn);

#endif
