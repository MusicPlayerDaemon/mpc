// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

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
