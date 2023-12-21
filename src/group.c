// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

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

