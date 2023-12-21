// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef MPC_STATUS_FORMAT_H
#define MPC_STATUS_FORMAT_H

#include "Compiler.h"

struct mpd_status;

/**
 * Returns percentage of elapsed time from current status
 *
 * @param status the status object
 * @return 100*elapsed_time/total duration of the song
 */
gcc_pure
unsigned
elapsed_percent(const struct mpd_status *status);

/**
 * Pretty-print data regarding current status using the given format
 * specification
 *
 * @param status the mpd_status object
 * @param format the format string
 * @return the resulting string to be freed by free(); NULL if
 * no format string group produced any output
 */
gcc_malloc
char *
format_status(const struct mpd_status *status, const char *format);

#endif
