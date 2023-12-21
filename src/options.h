// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

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
	const char *partition;
	int port;
	const char *password;
	const char *format;

	struct Range range;

	int verbosity; // 0 for quiet, 1 for default, 2 for verbose
	bool wait;

	bool custom_format;

	bool with_prio;
};


void print_option_help(void);
void parse_options(int * argc_p, char ** argv);

extern struct Options options;

#endif /* OPTIONS_H */
