// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef MPC_TAGS_H
#define MPC_TAGS_H

#include <stdbool.h>

struct mpd_connection;

/**
 * Send "tagtypes" to MPD, configuring the tags which are going to be
 * sent by MPD in following responses, based on the given format.
 *
 * @param c the MPD connection; it must be in "command list" mode
 * @param format the configured song format, or NULL to disable all
 * tags
 */
bool
send_tag_types_for_format(struct mpd_connection *c,
			  const char *format);

#endif
