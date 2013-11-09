/* music player command (mpc)
 * Copyright (C) 2003-2008 Warren Dukes <warren.dukes@gmail.com>,
				Eric Wong <normalperson@yhbt.net>,
				Daniel Brown <danb@cs.utexas.edu>
 * Copyright (C) 2008-2010 Max Kellermann <max@duempel.org>
 * Project homepage: http://musicpd.org
 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef MPC_UTIL_H
#define MPC_UTIL_H

#include <mpd/client.h>

struct mpd_connection;
struct mpd_song;

struct int_value_change {
        int value;
        bool is_relative;
};

void
printErrorAndExit(struct mpd_connection *conn);

/**
 * Call mpd_response_finish(), and if that fails, call
 * printErrorAndExit().
 */
void
my_finishCommand(struct mpd_connection *conn);

void free_pipe_array (int max, char ** array);
int stdinToArgArray(char *** array);
int get_boolean (const char * arg);

int parse_int(const char *, int *);
int parse_float(const char *, float *);
int parse_songnum(const char *, int *);
int parse_int_value_change(const char *, struct int_value_change *);

void
pretty_print_song(const struct mpd_song *song);

void
print_entity_list(struct mpd_connection *c, enum mpd_entity_type filter_type);

void
print_filenames(struct mpd_connection *conn);

#endif /* MPC_UTIL_H */	
