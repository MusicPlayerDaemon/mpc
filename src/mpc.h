// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef MPC_H
#define MPC_H

#include "config.h"

#define STDIN_SYMBOL	"-"

struct mpd_connection;

typedef int (*cmdhandler)(int argc, char **argv, struct mpd_connection *conn);

#endif /* MPC_H */
