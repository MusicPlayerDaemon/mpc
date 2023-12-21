// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project
#ifndef MPC_TAB_H
#define MPC_TAB_H

struct mpd_connection;

int
cmd_loadtab(int argc, char **argv, struct mpd_connection *conn);

int
cmd_lstab(int argc, char **argv, struct mpd_connection *conn);

int
cmd_tab(int argc, char **argv, struct mpd_connection *conn);

#endif
