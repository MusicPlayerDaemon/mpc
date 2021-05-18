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

#ifndef LIST_H
#define LIST_H

struct ListNode {
	/* data store in node */
	void * data;
	/* next node in list */
	struct ListNode *nextNode;
};

struct List {
	/* first node in list */
	struct ListNode *firstNode;
	/* last node in list */
	struct ListNode **tail;
	/* number of nodes */
	long numberOfNodes;
};

/* initializes the list
 */
void
makeList(struct List *list);

void
insertInListWithoutKey(struct List *list, void *data);

/* frees memory malloc'd for list and its nodes
 *  _list_ -> List to be free'd
 */
void
freeList(struct List *list);

#endif
