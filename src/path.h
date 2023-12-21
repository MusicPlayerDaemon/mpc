// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef MPC_PATH_H
#define MPC_PATH_H

#include "Compiler.h"

#include <stdbool.h>

struct mpd_connection;

bool
path_prepare(struct mpd_connection *conn);

/**
 * Convert an absolute path to one relative to the music directory.
 * That works only if we're connected to MPD via UNIX domain socket
 * and MPD supports the "config" command.
 */
gcc_pure
const char *
to_relative_path(const char *path);

#endif
