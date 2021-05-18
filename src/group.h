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
