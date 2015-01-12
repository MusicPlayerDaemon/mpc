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

#ifndef MPC_SONG_FORMAT_H
#define MPC_SONG_FORMAT_H

#include "Compiler.h"

struct mpd_song;

/**
 * Pretty-print song metadata into a string using the given format
 * specification.
 *
 * @param song the song object
 * @param format the format string
 * @return the resulting string to be freed by free(); NULL if
 * no format string group produced any output
 */
gcc_malloc
char *
format_song(const struct mpd_song *song,
	    const char *format);

#endif
