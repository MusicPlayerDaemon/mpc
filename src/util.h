/* music player command (mpc)
 * Copyright (C) 2003-2008 Warren Dukes <warren.dukes@gmail.com>,
				Eric Wong <normalperson@yhbt.net>,
				Daniel Brown <danb@cs.utexas.edu>
 * Copyright (C) 2008-2009 Max Kellermann <max@duempel.org>
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

#include "libmpdclient.h"

#define VALUE_CHANGE(type) \
struct type##_value_change { \
        type value; \
        int is_relative; \
};

VALUE_CHANGE(int) /* struct int_value_change */

typedef struct _Constraint {
	int type;
	char *query;
} Constraint;

void printErrorAndExit(mpd_Connection * conn);
void free_pipe_array (int max, char ** array);
int stdinToArgArray(char *** array);
int get_boolean (const char * arg);
int get_search_type(const char * arg);
int get_constraints(int argc, char **argv, Constraint **constraints);
int parse_int(const char *, int *);
int parse_songnum(const char *, int *);
int parse_int_value_change(const char *, struct int_value_change *);
void pretty_print_song(struct mpd_song *song);
void print_filenames(mpd_Connection *conn);

#endif /* MPC_UTIL_H */	
