/* mpc
 * (c) 2003-2004 by normalperson and Warren Dukes (warren.dukes@gmail.com)
 *              and Daniel Brown (danb@cs.utexas.edu)
 * This project's homepage is: http://www.musicpd.org
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

void printErrorAndExit(mpd_Connection * conn);
void free_pipe_array (int max, char ** array);
int stdinToArgArray(char *** array);
int get_boolean (const char * arg);
int parse_int(const char *, int *);
int parse_songnum(const char *, int *);
int parse_int_value_change(const char *, struct int_value_change *);
void pretty_print_song (mpd_Song * song);

#endif /* MPC_UTIL_H */	
