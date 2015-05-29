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

#ifndef MPC_UTIL_H
#define MPC_UTIL_H

#include <mpd/client.h>

struct mpd_connection;
struct mpd_song;

#define DIE(...) do { fprintf(stderr, __VA_ARGS__); return -1; } while(0)

#define SIMPLE_CMD(funcname, libmpdclient_funcname, ret) \
int funcname(gcc_unused int argc, gcc_unused char **argv, \
	     struct mpd_connection *conn) { \
	if (!libmpdclient_funcname(conn)) \
		printErrorAndExit(conn); \
	return ret; \
}

#define SIMPLE_ONEARG_CMD(funcname, libmpdclient_funcname, ret) \
int funcname (gcc_unused int argc, char **argv, struct mpd_connection *conn) { \
	if (!libmpdclient_funcname(conn, charset_to_utf8(argv[0]))) \
		printErrorAndExit(conn); \
	return ret; \
}

void
printErrorAndExit(struct mpd_connection *conn);

/**
 * Call mpd_response_finish(), and if that fails, call
 * printErrorAndExit().
 */
void
my_finishCommand(struct mpd_connection *conn);

struct mpd_status *
getStatus(struct mpd_connection *conn);

void
pretty_print_song(const struct mpd_song *song);

void
print_entity_list(struct mpd_connection *c, enum mpd_entity_type filter_type);

void
print_filenames(struct mpd_connection *conn);

#endif /* MPC_UTIL_H */	
