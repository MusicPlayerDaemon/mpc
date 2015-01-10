/*
 * music player command (mpc)
 * Copyright (C) 2003-2015 The Music Player Daemon Project
 * http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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
