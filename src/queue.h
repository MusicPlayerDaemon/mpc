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

#ifndef MPC_QUEUE_H
#define MPC_QUEUE_H

struct mpd_connection;

int
cmd_clear(int argc, char **argv, struct mpd_connection *conn);

int
cmd_shuffle(int argc, char **argv, struct mpd_connection *conn);

int
cmd_add(int argc, char **argv, struct mpd_connection *conn);

int
cmd_crop(int argc, char **argv, struct mpd_connection *conn);

int
cmd_del(int argc, char **argv, struct mpd_connection *conn);

int
cmd_playlist(int argc, char **argv, struct mpd_connection *conn);

int
cmd_insert(int argc, char **argv, struct mpd_connection *conn);

#endif /* COMMAND_H */
