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

#ifndef MPC_ARGS_H
#define MPC_ARGS_H

#include "Compiler.h"

#include <stdbool.h>

struct int_value_change {
	int value;
	bool is_relative;
};

int
stdinToArgArray(char ***array);

int
stdinAndPreambleToArgArray(char ***array, char *);

void
free_pipe_array(unsigned max, char **array);

gcc_pure
bool
contains_absolute_path(unsigned argc, char **argv);

gcc_pure
bool
contains_absolute_path_from(unsigned argc, char **argv, unsigned from);

void
strip_trailing_slash(char *s);

int
get_boolean(const char *arg);

/**
 * @return true on success
 */
bool
parse_unsigned(const char *s, unsigned *value_r);

/**
 * @return true on success
 */
bool
parse_int(const char *s, int *value_r);

/**
 * @return true on success
 */
bool
parse_float(const char *s, float *value_r);

/**
 * note - simply strips number out of formatting; does not -1 or +1 or
 * change the number in any other way for that matter
 *
 * @return true on success
 */
bool
parse_songnum(const char *s, int *value_r);

bool
parse_int_value_change(const char *s, struct int_value_change *value_r);

#endif
