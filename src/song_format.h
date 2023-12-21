// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

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
