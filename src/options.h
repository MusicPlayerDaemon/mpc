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

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

#define V_QUIET 0
#define V_DEFAULT 1
#define V_VERBOSE 2
#define F_DEFAULT \
    "[%name%: &[[%artist%|%performer%|%composer%|%albumartist%] - ]%title%]|%name%|[[%artist%|%performer%|%composer%|%albumartist%] - ]%title%|%file%"

struct Range {
	unsigned start, end;
};

struct Options {
	const char *host;
	const char *port_str;
	int port;
	const char *password;
	const char *format;

	struct Range range;

	int verbosity; // 0 for quiet, 1 for default, 2 for verbose
	bool wait;

	bool custom_format;
};


void print_option_help(void);
void parse_options(int * argc_p, char ** argv);

extern struct Options options;

#endif /* OPTIONS_H */
