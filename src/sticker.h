// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef MPC_STICKER_H
#define MPC_STICKER_H

struct mpd_connection;

int
cmd_sticker(int argc, char **argv, struct mpd_connection *conn);

#endif
