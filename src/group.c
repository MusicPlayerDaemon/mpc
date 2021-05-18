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

#include "group.h"

#include <stdio.h>
#include <string.h>

bool
mpc_groups_collect(struct mpc_groups *g, int *argc_p, char **argv)
{
	int argc = *argc_p;

	while (argc >= 2 && strcmp(argv[argc - 2], "group") == 0) {
		if (g->n_groups >= MAX_GROUPS) {
			fprintf(stderr, "Too many \"group\" parameters\n");
			return false;
		}

		const char *name = argv[argc - 1];
		enum mpd_tag_type t = mpd_tag_name_iparse(name);
		if (t == MPD_TAG_UNKNOWN) {
			fprintf(stderr, "Unknown tag: %s\n", name);
			return false;
		}

		if (mpc_groups_find(g, t) >= 0) {
			fprintf(stderr, "Duplicate group tag: %s\n", name);
			return false;
		}

		g->groups[g->n_groups++] = t;
		argc -= 2;
	}

	*argc_p = argc;
	return true;
}

bool
mpc_groups_send(struct mpd_connection *c, const struct mpc_groups *g)
{
	/* iterate the array in reverse order, because
	   mpc_groups_collect() was using reverse order as well */
	for (size_t i = g->n_groups; i > 0;)
		if (!mpd_search_add_group_tag(c, g->groups[--i]))
			return false;

	return true;
}

