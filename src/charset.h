// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef CHAR_CONV_H
#define CHAR_CONV_H

#include "config.h"
#include "Compiler.h"

#include <stdbool.h>

#ifdef HAVE_ICONV

/**
 * Initializes the character set conversion library.
 *
 * @param enable_input allow conversion from locale to UTF-8
 * @param enable_output allow conversion from UTF-8 to locale
 */
void
charset_init(bool enable_input, bool enable_output);

void charset_deinit(void);

gcc_pure
const char *
charset_to_utf8(const char *from);

gcc_pure
const char *
charset_from_utf8(const char *from);

#else

static inline void
charset_init(bool disable_input, bool disable_output)
{
	(void)disable_input;
	(void)disable_output;
}

static inline void
charset_deinit(void)
{
}

static inline const char *
charset_to_utf8(const char *from)
{
	return from;
}

static inline const char *
charset_from_utf8(const char *from)
{
	return from;
}

#endif

#endif
