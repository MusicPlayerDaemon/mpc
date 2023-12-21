// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

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
