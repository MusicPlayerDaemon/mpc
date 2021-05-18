/*
 * music player command (mpc)
 * Copyright 2003-2021 The Music Player Daemon Project
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
