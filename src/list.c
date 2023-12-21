// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "list.h"

#include <stdlib.h>
#include <assert.h>

void
makeList(struct List *list)
{
	assert(list!=NULL);

	list->firstNode = NULL;
	list->tail = &list->firstNode;
	list->numberOfNodes = 0;
}

void
insertInListWithoutKey(struct List *list, void * data) {
	struct ListNode *node;

	assert(list!=NULL);
	assert(data!=NULL);

	node = malloc(sizeof(*node));
	assert(node!=NULL);

	node->data = data;
	node->nextNode = NULL;

	*list->tail = node;
	list->tail = &node->nextNode;

	list->numberOfNodes++;
}

void
freeList(struct List *list)
{
	struct ListNode * tmpNode;
	struct ListNode * tmpNode2;

	assert(list!=NULL);

	tmpNode = list->firstNode;

	while(tmpNode!=NULL) {
		tmpNode2 = tmpNode->nextNode;
		free(tmpNode);
		tmpNode = tmpNode2;
	}
}
