// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "tags.h"
#include "format.h"

#include <mpd/client.h>

#include <stdint.h>
#include <stdlib.h>

static uint64_t tag_bits;

static const char *
collect_tags(gcc_unused const void *object, const char *name)
{
	enum mpd_tag_type tag_type = mpd_tag_name_iparse(name);
	if (tag_type != MPD_TAG_UNKNOWN && tag_type < 64)
		tag_bits |= (uint64_t)1 << (unsigned)tag_type;

	return NULL;
}

bool
send_tag_types_for_format(struct mpd_connection *c,
			  const char *format)
{
	if (format == NULL)
		return mpd_send_clear_tag_types(c);

	/* use format_object() to fill the "tag_bits" mask */

	tag_bits = 0;

	char *result = format_object(format, NULL, collect_tags);
	free(result);

	if (!mpd_send_clear_tag_types(c))
		return false;

	/* convert the "tag_bits" mask to an array of enum
	   mpd_tag_type for mpd_send_enable_tag_types() */

	enum mpd_tag_type types[64];
	unsigned n = 0;

	for (unsigned i = 0; i < 64; ++i)
		if (tag_bits & ((uint64_t)1 << i))
			types[n++] = (enum mpd_tag_type)i;

	return n == 0 || mpd_send_enable_tag_types(c, types, n);
}
