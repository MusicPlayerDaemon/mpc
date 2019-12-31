/*
 * music player command (mpc)
 * Copyright 2003-2018 The Music Player Daemon Project
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

#include "list.h"

#include <stdlib.h>
#include <assert.h>

struct List *
makeList(void)
{
	struct List *list = malloc(sizeof(*list));

	assert(list!=NULL);

	list->firstNode = NULL;
	list->lastNode = NULL;
	list->numberOfNodes = 0;

	return list;
}

void
insertInListWithoutKey(struct List *list, void * data) {
	struct ListNode *node;

	assert(list!=NULL);
	assert(data!=NULL);

	node = malloc(sizeof(*node));
	assert(node!=NULL);

	if(list->firstNode==NULL) {
		assert(list->lastNode==NULL);
		list->firstNode = node;
	}
	else {
		assert(list->lastNode!=NULL);
		assert(list->lastNode->nextNode==NULL);
		list->lastNode->nextNode = node;
	}

	node->data = data;
	node->nextNode = NULL;

	list->lastNode = node;

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

	free(list);
}
