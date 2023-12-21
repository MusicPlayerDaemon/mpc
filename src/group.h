// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#ifndef MPC_GROUP_H
#define MPC_GROUP_H

#include <mpd/client.h>

#include <stddef.h>

enum { MAX_GROUPS = 4 };

struct mpc_groups {
	enum mpd_tag_type groups[MAX_GROUPS];
	size_t n_groups;
};

static inline void
mpc_groups_init(struct mpc_groups *g)
{
	g->n_groups = 0;
}

static inline int
mpc_groups_find(const struct mpc_groups *g, enum mpd_tag_type t)
{
	for (size_t i = 0; i < g->n_groups; ++i)
		if (g->groups[i] == t)
			return i;

	return -1;
}

bool
mpc_groups_collect(struct mpc_groups *g, int *argc, char **argv);

bool
mpc_groups_send(struct mpd_connection *c, const struct mpc_groups *g);

#endif
